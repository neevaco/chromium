// Copyright Neeva. All rights reserved.

#include "weblayer/renderer/neeva/content_filter.h"

#include "base/strings/stringprintf.h"
#include "net/base/load_flags.h"
#include "services/network/public/cpp/resource_request.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/platform/web_url_request.h"

namespace weblayer {
namespace neeva {

ContentFilter::~ContentFilter() = default;

ContentFilter::ContentFilter(
    scoped_refptr<ContentFilteringAgent> agent, int render_frame_id,
    const blink::WebURLRequest& request)
    : agent_(std::move(agent)), render_frame_id_(render_frame_id) {
  auto top_frame_origin = request.TopFrameOrigin();
  if (top_frame_origin) {
    top_frame_origin_ = *top_frame_origin;
  }
  agent_->Log(
      base::StringPrintf("Created ContentFilter: render_frame_id=%d [top_origin=%s]",
          render_frame_id, top_frame_origin_.GetURL().spec().c_str()));
}

void ContentFilter::WillStartRequest(
    network::ResourceRequest* request, bool* defer) {
  (void) render_frame_id_;

  *defer = false;

  agent_->Log(
      base::StringPrintf("WillStartRequest: [%s] dest=%d",
          request->url.spec().c_str(), request->destination));

  switch (agent_->GetPolicyForRequest(request->url, top_frame_origin_)) {
    case ContentFilteringPolicy::kAllow:
      break;
    case ContentFilteringPolicy::kBlockCookies:
      request->load_flags |= net::LOAD_DO_NOT_SAVE_COOKIES;
      request->credentials_mode = network::mojom::CredentialsMode::kOmit;
      break;
    case ContentFilteringPolicy::kBlockRequest:
      delegate_->CancelWithError(net::ERR_ABORTED);
      break;
  }

  /*
  if (request->destination == network::mojom::RequestDestination::kImage) {
    if (delegate_) {
      delegate_->CancelWithError(net::ERR_ABORTED);
    }
  }
  */
}

}  // namespace neeva
}  // namespace weblayer
