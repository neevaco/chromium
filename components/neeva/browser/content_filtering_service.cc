// Copyright Neeva. All rights reserved.

#include "components/neeva/browser/content_filtering_service.h"

#include "base/logging.h"
#include "components/neeva/browser/content_filter_client.h"
#include "components/neeva/browser/content_filter_rules_config.h"
#include "components/neeva/browser/content_filter_stats.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

namespace neeva {

ContentFilteringService::ContentFilteringService(int render_process_id)
    : render_process_id_(render_process_id) {
}

ContentFilteringService::~ContentFilteringService() = default;

// static
void ContentFilteringService::AddInterface(
    service_manager::BinderRegistry* registry, int render_process_id) {
  auto create_service =
      [](int render_process_id,
         mojo::PendingReceiver<mojom::ContentFilteringService> receiver) {
        mojo::MakeSelfOwnedReceiver(
            std::make_unique<ContentFilteringService>(render_process_id),
            std::move(receiver));
      };
  registry->AddInterface(
      base::BindRepeating(create_service, render_process_id),
      content::GetUIThreadTaskRunner({}));
}

void ContentFilteringService::Log(const std::string& message) {
  LOG(ERROR) << ">>> " << message;
}

void ContentFilteringService::GetRulesProvider(
    mojo::PendingReceiver<mojom::ContentFilterRulesProvider> receiver) {
  auto* rph = content::RenderProcessHost::FromID(render_process_id_);
  if (!rph) {
    LOG(ERROR) << "No RenderProcessHost for ID";
    return;
  }
  ContentFilterRulesConfig::GetOrCreate(rph->GetBrowserContext())->AddReceiver(
      std::move(receiver));
}

void ContentFilteringService::OnContentFiltered(
    int32_t render_frame_id, mojom::ContentFilterActionPtr action) {
  auto* rfh =
      content::RenderFrameHost::FromID(render_process_id_, render_frame_id);
  if (!rfh) {
    LOG(ERROR) << ">>> No matching RenderFrameHost";
    return;
  }
  rfh = rfh->GetMainFrame();

  ContentFilterStats::GetOrCreateForCurrentDocument(rfh)->RecordFilteredHost(
      action->host);

  auto* web_contents = content::WebContents::FromRenderFrameHost(rfh);
  if (web_contents) {
    auto* client = ContentFilterClient::Get(web_contents);
    if (client) {
      client->Notify();
    }
  }
}

}  // namespace neeva
