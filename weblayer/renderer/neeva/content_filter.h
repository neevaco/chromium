// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_RENDERER_NEEVA_CONTENT_FILTER_H__
#define WEBLAYER_RENDERER_NEEVA_CONTENT_FILTER_H__

#include "third_party/blink/public/common/loader/url_loader_throttle.h"
#include "url/origin.h"
#include "weblayer/renderer/neeva/content_filtering_agent.h"

namespace blink {
class WebURLRequest;
}

namespace weblayer {
namespace neeva {

class ContentFilter : public blink::URLLoaderThrottle {
 public:
  ~ContentFilter() override;
  ContentFilter(
      scoped_refptr<ContentFilteringAgent> agent, int render_frame_id,
      const blink::WebURLRequest& request);

  // blink::URLLoaderThrottle overrides:
  void WillStartRequest(network::ResourceRequest* request, bool* defer) override;

 private:
  scoped_refptr<ContentFilteringAgent> agent_;
  int render_frame_id_;
  url::Origin top_frame_origin_;
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_RENDERER_NEEVA_CONTENT_FILTER_H__
