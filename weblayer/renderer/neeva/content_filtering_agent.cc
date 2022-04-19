// Copyright Neeva. All rights reserved.

#include "weblayer/renderer/neeva/content_filtering_agent.h"

#include "base/threading/sequenced_task_runner_handle.h"
#include "third_party/blink/public/common/thread_safe_browser_interface_broker_proxy.h"
#include "weblayer/renderer/neeva/content_filter.h"

namespace weblayer {
namespace neeva {

ContentFilteringAgent::SharedState::SharedState(
    blink::ThreadSafeBrowserInterfaceBrokerProxy* broker) 
    : task_runner_(base::SequencedTaskRunnerHandle::Get()) {
  broker->GetInterface(service_.BindNewPipeAndPassReceiver());
}

void ContentFilteringAgent::SharedState::DeleteOnCorrectThread() const {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    // NOTE: This is only called when there are no more references to
    // |this|, so binding it unretained is both safe and necessary.
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&SharedState::DeleteOnCorrectThread,
                                  base::Unretained(this)));
  } else {
    delete this;
  }
}

void ContentFilteringAgent::SharedState::Log(const std::string& message) {
  if (task_runner_->RunsTasksInCurrentSequence()) {
    service_->Log(message);
  } else {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&SharedState::Log, this, message));
  }
}

ContentFilteringAgent::SharedState::~SharedState() = default;

ContentFilteringAgent::~ContentFilteringAgent() = default;

ContentFilteringAgent::ContentFilteringAgent(blink::ThreadSafeBrowserInterfaceBrokerProxy* broker)
  : shared_state_(base::MakeRefCounted<SharedState>(broker)) {
}

ContentFilteringAgent::ContentFilteringAgent(const ContentFilteringAgent& other) = default;
ContentFilteringAgent::ContentFilteringAgent(ContentFilteringAgent&& other) = default;
ContentFilteringAgent& ContentFilteringAgent::operator=(const ContentFilteringAgent& other) = default;
ContentFilteringAgent& ContentFilteringAgent::operator=(ContentFilteringAgent&& other) = default;

std::unique_ptr<blink::URLLoaderThrottle> ContentFilteringAgent::CreateThrottle(
    int render_frame_id, const blink::WebURLRequest& request) {
  return std::make_unique<ContentFilter>(*this, render_frame_id, request);
}

void ContentFilteringAgent::Log(const std::string& message) {
  shared_state_->Log(message);
}

}  // namespace neeva
}  // namespace weblayer
