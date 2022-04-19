// Copyright Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filtering_service.h"

#include "base/logging.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

namespace weblayer {
namespace neeva {

ContentFilteringService::ContentFilteringService() = default;

ContentFilteringService::~ContentFilteringService() = default;

// static
void ContentFilteringService::AddInterface(service_manager::BinderRegistry* registry) {
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

}  // namespace neeva
}  // namespace weblayer
