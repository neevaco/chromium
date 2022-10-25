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
// Creates the Service. Called in weblayer/browser/content_browser_client_impl.cc
void ContentFilteringService::AddInterface(
    service_manager::BinderRegistry* registry, int render_process_id) {
  auto create_service =
      [](int render_process_id,
         mojo::PendingReceiver<mojom::ContentFilteringService> receiver) {
        mojo::MakeSelfOwnedReceiver( // observes if the pipe is broken and cleans it up if it is
            std::make_unique<ContentFilteringService>(render_process_id),
            std::move(receiver));
      };
  registry->AddInterface(
      base::BindRepeating(create_service, render_process_id),
      content::GetUIThreadTaskRunner({}));
}

// Called by renderer/content_filtering_agent constructor
void ContentFilteringService::AddRulesListener(
    mojo::PendingRemote<mojom::ContentFilterRulesListener> remote) {
  auto* rph = content::RenderProcessHost::FromID(render_process_id_);
  if (!rph) {
    LOG(ERROR) << "No RenderProcessHost for ID";
    return;
  }
  // Note that unlike AddInterface (above), this is given only a remote.
  // The renderer only cares about new rule updates
  // Also ties the lifecycle of a ContentFilterRulesConfig with 1 browser context. 
  ContentFilterRulesConfig::GetOrCreate(rph->GetBrowserContext())->AddListener(
      std::move(remote));
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
  
  // Updates ContentFilterStats
  ContentFilterStats::GetOrCreateForCurrentDocument(rfh)->RecordFilteredHost(
      action->rules_name, action->host);

  // Notify the browser that new ContentFilterStats are ready. 
  auto* web_contents = content::WebContents::FromRenderFrameHost(rfh);
  if (web_contents) {
    auto* client = ContentFilterClient::Get(web_contents);
    // Get the client (aka. browserContext)
    if (client) { // if exists -> tell it OnContentFiltered()
      client->Notify();
    }
  }
}

}  // namespace neeva
