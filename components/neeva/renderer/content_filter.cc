// Copyright Neeva. All rights reserved.

#include "components/neeva/renderer/content_filter.h"

#include "base/strings/stringprintf.h"
#include "net/base/load_flags.h"
#include "services/network/public/cpp/resource_request.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/platform/web_url_request.h"

using namespace url_pattern_index;

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
  // TopFrameOrigin can sometimes be null. It appears that in those cases,
  // WillStartRequest is not called, so in practice it shouldn't be an issue.
  auto top_frame_origin = request.TopFrameOrigin();
  if (top_frame_origin) {
    top_frame_origin_ = *top_frame_origin;
  }
}

void ContentFilter::WillStartRequest(
    network::ResourceRequest* request, bool* defer) {
  *defer = false;

  auto element_type = GetElementTypeForRequest(request_context_type_);

  std::string rules_name;
  switch (agent_->GetPolicyForRequest(
      request->url, top_frame_origin_, element_type, &rules_name)) {
    case ContentFilteringPolicy::kAllow:
      return;
    case ContentFilteringPolicy::kBlockCookies:
      // TODO: Confirm this prevents sending cookies.
      request->credentials_mode = network::mojom::CredentialsMode::kOmit;
      delegate_->RestartWithFlags(net::LOAD_DO_NOT_SAVE_COOKIES);
      break;
    case ContentFilteringPolicy::kBlockRequest:
      delegate_->CancelWithError(net::ERR_ABORTED);
      break;
  }

  // Report content filtering.
  mojom::ContentFilterActionPtr action(mojom::ContentFilterAction::New());
  action->rules_name = rules_name;
  action->host = request->url.host();
  agent_->OnContentFiltered(render_frame_id_, std::move(action));
}

}  // namespace neeva
