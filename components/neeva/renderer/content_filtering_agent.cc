// Copyright Neeva. All rights reserved.

#include "components/neeva/renderer/content_filtering_agent.h"

#include "base/files/memory_mapped_file.h"
#include "base/strings/stringprintf.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/time/time.h"
#include "components/neeva/flat/content_filter_rules_generated.h"
#include "components/neeva/renderer/content_filter.h"
#include "components/neeva/renderer/css_rule_list_matcher.h"
#include "components/url_pattern_index/url_pattern_index.h"
#include "content/public/renderer/render_frame.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/platform/web_url.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"

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

  // See ContentFilteringService::AddInterface() for the handling of the reciever part. 
  broker->GetInterface(service_.BindNewPipeAndPassReceiver());

  // Allows the ContentFilterConfig to call OnReceiveNewRules() and 
  // for the service to "receive" those callbacks. 
  service_->AddRulesListener(receiver_.BindNewPipeAndPassRemote());
}

// Creates ContentFilter objects. 
// Gets called by WebLayer (weblayer/renderer/url_loader_throttle_provider.cc) 
std::unique_ptr<blink::URLLoaderThrottle> ContentFilteringAgent::CreateThrottle(
    int render_frame_id, const blink::WebURLRequest& request) {
  return std::make_unique<ContentFilter>(
      base::WrapRefCounted(this), render_frame_id, request);
}

void ContentFilteringAgent::RunScriptsAtDocumentStart(
    content::RenderFrame* render_frame) {
  auto* web_frame = render_frame->GetWebFrame();
  if (!web_frame)
    return;

  // Use the security origin here instead of the URL to cover script generated
  // documents as well.
  std::string host = web_frame->GetDocument().GetSecurityOrigin().Host().Utf8();

  std::vector<std::string> stylesheets;
  {
    // Protect access to |rules_| and |filters_|. NOTE: This should only be
    // called on the same thread that mutates these objects, so this lock
    // shouldn't be necessary, but guard access for consistency.
    base::AutoLock locked(rules_lock_);

    if (!rules_ || rules_->mode == mojom::ContentFilterMode::BLOCK_COOKIES)
      return;

    for (const auto& filter : filters_) {
      if (!filter.css_matcher)
        continue;

      std::string stylesheet = filter.css_matcher->GetStyleSheetForHost(host);
      if (stylesheet.empty())
        continue;

      // Queue up the stylesheets here to minimize what code gets run while
      // holding |rules_lock_|.
      stylesheets.push_back(std::move(stylesheet));
    }
  }

  for (const auto& stylesheet : stylesheets) {
    web_frame->GetDocument().InsertStyleSheet(
        blink::WebString::FromUTF8(stylesheet), nullptr,
        blink::WebCssOrigin::kUser);
  }
}

void ContentFilteringAgent::OnContentFiltered(
    int32_t render_frame_id, mojom::ContentFilterActionPtr action) {
  // Any actions/communication to mojom ContentFilterService must be 
  // ran on the same thread as the agent/service.
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
    proto::ElementType element_type, std::string* rules_name) const {
  // NOTE: Called from any thread.
  base::AutoLock locked(rules_lock_);

  if (!rules_) {
    return ContentFilteringPolicy::kAllow;
  }

  // Apply top-level host exclusions.
  // TODO: Use a set for more efficient lookup.
  auto end = rules_->top_level_host_exclusions.end();
  if (std::find(rules_->top_level_host_exclusions.begin(), end,
                first_party_origin.host()) != end) {
    return ContentFilteringPolicy::kAllow;
  }

  bool is_url_third_party = IsThirdParty(url, first_party_origin);

  for (const auto& filter : filters_) {
    if (!filter.url_matcher)
      continue;

    if (!filter.url_matcher->FindMatch(
            url, first_party_origin, element_type,
            proto::ACTIVATION_TYPE_UNSPECIFIED,
            is_url_third_party,
            false,
            UrlPatternIndexMatcher::EmbedderConditionsMatcher(),
            UrlPatternIndexMatcher::FindRuleStrategy::kAny)) {
      continue;
    }

    // A match was found!
    *rules_name = filter.rules_name;
    // There's no point in blocking foo.com from making requests from first-party origins (e.g. foo.com). 
    // Doing so can break logins or other site functionality. 
    if (!is_url_third_party) {
      // Downgrade from kBlockRequests to kBlockCookies to minimize impact on sites. 
      return ContentFilteringPolicy::kBlockCookies;
    }

    switch (rules_->mode) {
      case mojom::ContentFilterMode::BLOCK_COOKIES:
        return ContentFilteringPolicy::kBlockCookies;
      case mojom::ContentFilterMode::BLOCK_REQUESTS:
        return ContentFilteringPolicy::kBlockRequest;
    }
  }

  return ContentFilteringPolicy::kAllow;
}

ContentFilteringAgent::Filter::Filter() = default;

ContentFilteringAgent::Filter::~Filter() = default;

ContentFilteringAgent::Filter::Filter(Filter&&) = default;

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

  filters_.clear();

  rules_ = std::move(new_rules);
  if (!rules_)
    return;

  for (auto& data : rules_->data) {
    Filter filter;
    filter.data = MapRegion(std::move(data->rules_data_fd),
                            data->rules_data_offset,
                            data->rules_data_size);
    if (filter.data) {
      const auto* flat_rules = flat::GetContentFilterRules(filter.data->data());
      filter.url_matcher = std::make_unique<UrlPatternIndexMatcher>(
          flat_rules->url_pattern_index());
      filter.css_matcher = std::make_unique<CssRuleListMatcher>(
          flat_rules->css_rule_list());
      filter.rules_name = data->rules_name;
      filters_.push_back(std::move(filter));
    } else {
      LOG(ERROR) << "Mapping the region failed!";
    }
  }
}

}  // namespace neeva
