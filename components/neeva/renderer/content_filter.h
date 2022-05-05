// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_RENDERER_CONTENT_FILTER_H__
#define COMPONENTS_NEEVA_RENDERER_CONTENT_FILTER_H__

#include "components/neeva/renderer/content_filtering_agent.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"
#include "third_party/blink/public/mojom/fetch/fetch_api_request.mojom.h"
#include "url/origin.h"

namespace blink {
class WebURLRequest;
}

namespace neeva {

// An instance of this class is created per resource request.
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
  blink::mojom::RequestContextType request_context_type_;
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_RENDERER_CONTENT_FILTER_H__
