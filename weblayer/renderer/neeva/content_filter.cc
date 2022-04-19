// Copyright Neeva. All rights reserved.

#include "weblayer/renderer/neeva/content_filter.h"

#include "base/strings/stringprintf.h"
#include "services/network/public/cpp/resource_request.h"

namespace weblayer {
namespace neeva {

ContentFilter::~ContentFilter() = default;

ContentFilter::ContentFilter(const ContentFilteringAgent& agent, int render_frame_id)
    : agent_(agent), render_frame_id_(render_frame_id) {}

void ContentFilter::WillStartRequest(
    network::ResourceRequest* request, bool* defer) {
  (void) render_frame_id_;

  *defer = false;

  //LOG(ERROR) << ">>> neeva::ContentFilter::WillStartRequest [" << request->url.spec() << "], destination=" << request->destination;

  agent_.Log(
      base::StringPrintf("WillStartRequest: [%s] dest=%d",
          request->url.spec().c_str(), request->destination));

  if (request->destination == network::mojom::RequestDestination::kImage) {
    if (delegate_) {
      delegate_->CancelWithError(net::ERR_ABORTED);
    }
  }
}

}  // namespace neeva
}  // namespace weblayer
