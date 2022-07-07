// Copyright Neeva. All rights reserved.

#include "components/neeva/renderer/content_filtering_agent.h"

#include "base/files/memory_mapped_file.h"
#include "base/strings/stringprintf.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "components/neeva/renderer/content_filter.h"
#include "components/url_pattern_index/url_pattern_index.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"

using namespace url_pattern_index;

namespace neeva {
namespace {

bool IsThirdParty(const GURL& url, const url::Origin& first_party_origin) {
  return first_party_origin.opaque() ||
    !net::registry_controlled_domains::SameDomainOrHost(
      url, first_party_origin,
      net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
}

std::unique_ptr<base::MemoryMappedFile> MapRegion(
    mojo::PlatformHandle data_handle,
    uint64_t offset,
    uint64_t size) {
  base::MemoryMappedFile::Region region;
  region.offset = static_cast<int64_t>(offset);
  region.size = static_cast<size_t>(size);

  auto memory_mapped_file = std::make_unique<base::MemoryMappedFile>();
  if (!memory_mapped_file->Initialize(base::File(data_handle.TakeFD()), region))
    return nullptr;

  return memory_mapped_file;
}

}  // namespace

ContentFilteringAgent::ContentFilteringAgent(
    blink::ThreadSafeBrowserInterfaceBrokerProxy* broker)
    : task_runner_(base::SequencedTaskRunnerHandle::Get()) {
  broker->GetInterface(service_.BindNewPipeAndPassReceiver());

  service_->AddRulesListener(receiver_.BindNewPipeAndPassRemote());
}

std::unique_ptr<blink::URLLoaderThrottle> ContentFilteringAgent::CreateThrottle(
    int render_frame_id, const blink::WebURLRequest& request) {
  return std::make_unique<ContentFilter>(
      base::WrapRefCounted(this), render_frame_id, request);
}

void ContentFilteringAgent::OnContentFiltered(
    int32_t render_frame_id, mojom::ContentFilterActionPtr action) {
  if (task_runner_->RunsTasksInCurrentSequence()) {
    service_->OnContentFiltered(render_frame_id, std::move(action));
  } else {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(
            &ContentFilteringAgent::OnContentFiltered,
            this, render_frame_id, std::move(action)));
  }
}

ContentFilteringPolicy ContentFilteringAgent::GetPolicyForRequest(
    const GURL& url, const url::Origin& first_party_origin,
    proto::ElementType element_type) const {
  // NOTE: Called from any thread.
  base::AutoLock locked(rules_lock_);

  if (!matcher_) {
    return ContentFilteringPolicy::kAllow;
  }

  // Apply top-level host exclusions.
  // TODO: Use a set for more efficient lookup.
  auto end = rules_->top_level_host_exclusions.end();
  if (std::find(rules_->top_level_host_exclusions.begin(), end,
                first_party_origin.host()) != end) {
    return ContentFilteringPolicy::kAllow;
  }

  if (!matcher_->FindMatch(
          url, first_party_origin, element_type,
          proto::ACTIVATION_TYPE_UNSPECIFIED,
          IsThirdParty(url, first_party_origin),
          false,
          UrlPatternIndexMatcher::EmbedderConditionsMatcher(),
          UrlPatternIndexMatcher::FindRuleStrategy::kAny)) {
    return ContentFilteringPolicy::kAllow;
  }

  // A match was found!
  switch (rules_->mode) {
    case mojom::ContentFilterMode::BLOCK_COOKIES:
      return ContentFilteringPolicy::kBlockCookies;
    case mojom::ContentFilterMode::BLOCK_REQUESTS:
      return ContentFilteringPolicy::kBlockRequest;
  }
}

ContentFilteringAgent::~ContentFilteringAgent() = default;

void ContentFilteringAgent::DeleteOnCorrectThread() const {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    // NOTE: This is only called when there are no more references to
    // |this|, so binding it unretained is both safe and necessary.
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&ContentFilteringAgent::DeleteOnCorrectThread,
                                  base::Unretained(this)));
  } else {
    delete this;
  }
}

void ContentFilteringAgent::OnReceiveNewRules(
    mojom::ContentFilterRulesPtr new_rules) {
  // Update the matcher.
  base::AutoLock locked(rules_lock_);

  matcher_.reset();
  rules_data_.reset();

  rules_ = std::move(new_rules);
  if (rules_) {
    rules_data_ = MapRegion(std::move(rules_->rules_data_fd),
                            rules_->rules_data_offset,
                            rules_->rules_data_size);
    if (rules_data_) {
      matcher_ = std::make_unique<UrlPatternIndexMatcher>(
          flat::GetUrlPatternIndex(rules_data_->data()));
    } else {
      LOG(ERROR) << "Mapping the region failed!";
    }
  }
}

}  // namespace neeva
