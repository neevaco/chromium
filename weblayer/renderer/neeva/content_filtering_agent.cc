// Copyright Neeva. All rights reserved.

#include "weblayer/renderer/neeva/content_filtering_agent.h"

#include "base/threading/sequenced_task_runner_handle.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "weblayer/renderer/neeva/content_filter.h"

namespace weblayer {
namespace neeva {

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

ContentFilteringPolicy ContentFilteringAgent::GetPolicyForRequest(
    const GURL& url, const url::Origin& first_party_origin) const {
  // TODO: Implement me!
  return ContentFilteringPolicy::kAllow;
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
  rules_ = std::move(new_rules);
  // TODO: Perform any other one-time setup for rules.
  RefreshRules();
}

}  // namespace neeva
}  // namespace weblayer
