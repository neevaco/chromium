// Copyright Neeva. All rights reserved.

#include "weblayer/renderer/neeva/content_filter.h"

#include "base/strings/stringprintf.h"
#include "net/base/load_flags.h"
#include "services/network/public/cpp/resource_request.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/platform/web_url_request.h"

using namespace url_pattern_index;

namespace weblayer {
namespace neeva {

namespace {

proto::ElementType GetElementTypeForRequest(
    blink::mojom::RequestContextType type) {
  proto::ElementType result;
  switch (type) {
    case blink::mojom::RequestContextType::SCRIPT:
      result = proto::ELEMENT_TYPE_SCRIPT;
      break;
    case blink::mojom::RequestContextType::IMAGE:
      result = proto::ELEMENT_TYPE_IMAGE;
      break;
    case blink::mojom::RequestContextType::STYLE:
      result = proto::ELEMENT_TYPE_STYLESHEET;
      break;
    case blink::mojom::RequestContextType::EMBED:
    case blink::mojom::RequestContextType::OBJECT:
      result = proto::ELEMENT_TYPE_OBJECT;
      break;
    case blink::mojom::RequestContextType::FETCH:
    case blink::mojom::RequestContextType::XML_HTTP_REQUEST:
      result = proto::ELEMENT_TYPE_XMLHTTPREQUEST;
      break;
    case blink::mojom::RequestContextType::FRAME:
    case blink::mojom::RequestContextType::IFRAME:
      result = proto::ELEMENT_TYPE_SUBDOCUMENT;
      break;
    case blink::mojom::RequestContextType::BEACON:
    case blink::mojom::RequestContextType::PING:
      result = proto::ELEMENT_TYPE_PING;
      break;
    case blink::mojom::RequestContextType::AUDIO:
    case blink::mojom::RequestContextType::VIDEO:
      result = proto::ELEMENT_TYPE_MEDIA;
      break;
    case blink::mojom::RequestContextType::FONT:
      result = proto::ELEMENT_TYPE_FONT;
      break;
    // Not sure how to support:
    //   ELEMENT_TYPE_OBJECT_SUBREQUEST
    //   ELEMENT_TYPE_POPUP
    //   ELEMENT_TYPE_WEBSOCKET
    //   ELEMENT_TYPE_WEBTRANSPORT
    //   ELEMENT_TYPE_WEBBUNDLE
    default:
      result = proto::ELEMENT_TYPE_UNSPECIFIED;
      break;
  }
  return result;
}

}  // namespace

ContentFilter::~ContentFilter() = default;

ContentFilter::ContentFilter(
    scoped_refptr<ContentFilteringAgent> agent, int render_frame_id,
    const blink::WebURLRequest& request)
    : agent_(std::move(agent)),
      render_frame_id_(render_frame_id),
      request_context_type_(request.GetRequestContext()) {
  auto top_frame_origin = request.TopFrameOrigin();
  if (top_frame_origin) {
    top_frame_origin_ = *top_frame_origin;
  }
  // Debug
  agent_->Log(
      base::StringPrintf("Created ContentFilter: render_frame_id=%d [top_origin=%s]",
          render_frame_id, top_frame_origin_.GetURL().spec().c_str()));
}

void ContentFilter::WillStartRequest(
    network::ResourceRequest* request, bool* defer) {
  *defer = false;

  // Debug
  agent_->Log(
      base::StringPrintf("WillStartRequest: [%s] dest=%d",
          request->url.spec().c_str(), request->destination));

  auto element_type = GetElementTypeForRequest(request_context_type_);

  switch (agent_->GetPolicyForRequest(request->url, top_frame_origin_, element_type)) {
    case ContentFilteringPolicy::kAllow:
      return;
    case ContentFilteringPolicy::kBlockCookies:
      // TODO: Confirm that this actually works at blocking cookies. Do we need
      // to call RestartWithFlags or does twiddling flags directly here work?
      request->load_flags |= net::LOAD_DO_NOT_SAVE_COOKIES;
      request->credentials_mode = network::mojom::CredentialsMode::kOmit;
      break;
    case ContentFilteringPolicy::kBlockRequest:
      delegate_->CancelWithError(net::ERR_ABORTED);
      break;
  }

  // Report content filtering.
  mojom::ContentFilterActionPtr action(mojom::ContentFilterAction::New());
  action->host = request->url.host();
  action->top_frame_host = top_frame_origin_.host();
  agent_->OnContentFiltered(render_frame_id_, std::move(action));
}

}  // namespace neeva
}  // namespace weblayer
