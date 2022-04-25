// Copyright Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filtering_service.h"

#include "base/logging.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "weblayer/browser/neeva/content_filter_rules_provider.h"

namespace weblayer {
namespace neeva {

ContentFilteringService::ContentFilteringService() = default;

ContentFilteringService::~ContentFilteringService() = default;

// static
void ContentFilteringService::AddInterface(
    service_manager::BinderRegistry* registry) {
  auto create_service =
      [](mojo::PendingReceiver<mojom::ContentFilteringService> receiver) {
        mojo::MakeSelfOwnedReceiver(std::make_unique<ContentFilteringService>(),
                                    std::move(receiver));
      };
  registry->AddInterface(
      base::BindRepeating(create_service), content::GetUIThreadTaskRunner({}));
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
  // TODO: implement me!
  LOG(ERROR) << ">>> BLOCKED: " << action->host;
}

}  // namespace neeva
}  // namespace weblayer
