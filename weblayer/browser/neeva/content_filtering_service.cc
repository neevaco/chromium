// Copyright Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filtering_service.h"

#include "base/logging.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "weblayer/browser/neeva/content_filter_rules_provider.h"
#include "weblayer/browser/neeva/content_filter_stats.h"

namespace weblayer {
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
  mojo::MakeSelfOwnedReceiver(std::make_unique<ContentFilterRulesProvider>(),
                              std::move(receiver));
}

void ContentFilteringService::OnContentFiltered(
    int32_t render_frame_id, mojom::ContentFilterActionPtr action) {
  LOG(ERROR) << ">>> BLOCKED: " << action->host;

  auto* rfh =
      content::RenderFrameHost::FromID(render_process_id_, render_frame_id);
  if (!rfh) {
    LOG(ERROR) << ">>> No matching RenderFrameHost";
    return;
  }
  rfh = rfh->GetMainFrame();

  ContentFilterStats::GetOrCreateForCurrentDocument(rfh)->RecordFilteredHost(
      action->host);

  // TODO: send notification
}

}  // namespace neeva
}  // namespace weblayer
