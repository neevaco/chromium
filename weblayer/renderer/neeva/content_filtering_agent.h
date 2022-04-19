// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_RENDERER_NEEVA_NEEVA_CONTENT_FILTERING_AGENT_H__
#define WEBLAYER_RENDERER_NEEVA_NEEVA_CONTENT_FILTERING_AGENT_H__

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

class ContentFilteringAgent {
 public:
  ~ContentFilteringAgent();
  explicit ContentFilteringAgent(blink::ThreadSafeBrowserInterfaceBrokerProxy* broker);

  ContentFilteringAgent(const ContentFilteringAgent& other);
  ContentFilteringAgent(ContentFilteringAgent&& other);
  ContentFilteringAgent& operator=(const ContentFilteringAgent& other);
  ContentFilteringAgent& operator=(ContentFilteringAgent&& other);

  std::unique_ptr<blink::URLLoaderThrottle> CreateThrottle(
      int render_frame_id, const blink::WebURLRequest& request);

  void Log(const std::string& message);

 private:
  struct SharedStateDeleter;

  class SharedState : public base::RefCountedThreadSafe<SharedState, SharedStateDeleter> {
   public:
    explicit SharedState(blink::ThreadSafeBrowserInterfaceBrokerProxy* broker);
    void DeleteOnCorrectThread() const;
    void Log(const std::string& message);
   private:
    friend struct Deleter;
    ~SharedState();
    scoped_refptr<base::SequencedTaskRunner> task_runner_;
    mojo::Remote<mojom::ContentFilteringService> service_;
  };

  struct SharedStateDeleter {
    static void Destruct(const SharedState* shared_state) {
      shared_state->DeleteOnCorrectThread();
    }
  };

  scoped_refptr<SharedState> shared_state_;
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_RENDERER_NEEVA_NEEVA_CONTENT_FILTERING_AGENT_H__
