// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_RENDERER_NEEVA_CONTENT_FILTERING_AGENT_H__
#define WEBLAYER_RENDERER_NEEVA_CONTENT_FILTERING_AGENT_H__

#include "base/memory/ref_counted.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace blink {
class ThreadSafeBrowserInterfaceBrokerProxy;
class URLLoaderThrottle;
class WebURLRequest;
}

namespace weblayer {
namespace neeva {

struct ContentFilteringAgentDeleter;

class ContentFilteringAgent
    : public base::RefCountedThreadSafe<ContentFilteringAgent,
                                        ContentFilteringAgentDeleter> {
 public:
  explicit ContentFilteringAgent(
      blink::ThreadSafeBrowserInterfaceBrokerProxy* broker);

  std::unique_ptr<blink::URLLoaderThrottle> CreateThrottle(
      int render_frame_id, const blink::WebURLRequest& request);

  void Log(const std::string& message);

  // TODO: Add method here to determine policy for a given request.
  // TODO: Add method to report filtered content.
  // Both of the above methods could be called on a background thread.

 private:
  friend struct ContentFilteringAgentDeleter;

  ~ContentFilteringAgent();
  void DeleteOnCorrectThread() const;
  void RefreshRules();
  void OnApplyNewRules(
      int64_t new_generation_num, mojom::ContentFilterRulesPtr new_rules);

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  mojo::Remote<mojom::ContentFilteringService> service_;
  mojom::ContentFilterRulesPtr rules_;
  int64_t current_generation_num_ = 0;
};

struct ContentFilteringAgentDeleter {
  static void Destruct(const ContentFilteringAgent* agent) {
    agent->DeleteOnCorrectThread();
  }
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_RENDERER_NEEVA_CONTENT_FILTERING_AGENT_H__
