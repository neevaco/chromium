// Copyright Neeva. All rights reserved.

#include "weblayer/renderer/neeva/content_filtering_agent.h"

#include "base/threading/sequenced_task_runner_handle.h"
#include "components/url_pattern_index/url_pattern_index.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "weblayer/renderer/neeva/content_filter.h"

namespace weblayer {
namespace neeva {

namespace {

bool IsThirdParty(const GURL& url, const url::Origin& first_party_origin) {
  return first_party_origin.opaque() ||
    !net::registry_controlled_domains::SameDomainOrHost(
      url, first_party_origin,
      net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
}

}  // namespace

ContentFilteringAgent::ContentFilteringAgent(
    blink::ThreadSafeBrowserInterfaceBrokerProxy* broker)
    : task_runner_(base::SequencedTaskRunnerHandle::Get()) {
  broker->GetInterface(service_.BindNewPipeAndPassReceiver());
  RefreshRules();
}

std::unique_ptr<blink::URLLoaderThrottle> ContentFilteringAgent::CreateThrottle(
    int render_frame_id, const blink::WebURLRequest& request) {
  return std::make_unique<ContentFilter>(
      base::WrapRefCounted(this), render_frame_id, request);
}

void ContentFilteringAgent::Log(const std::string& message) {
  if (task_runner_->RunsTasksInCurrentSequence()) {
    service_->Log(message);
  } else {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&ContentFilteringAgent::Log, this, message));
  }
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
    const GURL& url, const url::Origin& first_party_origin) const {
  // NOTE: Called from any thread.
  base::AutoLock locked(rules_lock_);

  if (!matcher_) {
    return ContentFilteringPolicy::kAllow;
  }

  // TODO: Plumb through element type.
  if (!matcher_->FindMatch(
          url, first_party_origin,
          url_pattern_index::proto::ELEMENT_TYPE_UNSPECIFIED,
          url_pattern_index::proto::ACTIVATION_TYPE_UNSPECIFIED,
          IsThirdParty(url, first_party_origin),
          false,
          url_pattern_index::UrlPatternIndexMatcher::EmbedderConditionsMatcher(),
          url_pattern_index::UrlPatternIndexMatcher::FindRuleStrategy::kAny)) {
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

void ContentFilteringAgent::RefreshRules() {
  service_->RefreshRules(
      current_generation_num_,
      base::BindOnce(&ContentFilteringAgent::OnApplyNewRules, this));
}

void ContentFilteringAgent::OnApplyNewRules(
    int64_t new_generation_num, mojom::ContentFilterRulesPtr new_rules) {
  current_generation_num_ = new_generation_num;

  // Update the matcher.
  {
    base::AutoLock locked(rules_lock_);

    matcher_.reset();
    rules_data_mapping_.reset();

    rules_ = std::move(new_rules);

    rules_data_mapping_ =
        rules_->url_pattern_data->Map(rules_->url_pattern_data->GetSize());

    const url_pattern_index::flat::UrlPatternIndex* flat_index =
        url_pattern_index::flat::GetUrlPatternIndex(rules_data_mapping_.get());
    matcher_ =
        std::make_unique<url_pattern_index::UrlPatternIndexMatcher>(flat_index);
  }

  // Kick-off another hanging refresh, waiting for the browser-side to let us know
  // when it has new rules for us.
  RefreshRules();
}

}  // namespace neeva
}  // namespace weblayer
