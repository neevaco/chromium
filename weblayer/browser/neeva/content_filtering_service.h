// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTERING_SERVICE_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTERING_SERVICE_H_

#include "services/service_manager/public/cpp/binder_registry.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace weblayer {
namespace neeva {

class ContentFilteringService : public mojom::ContentFilteringService {
 public:
  ContentFilteringService();
  ~ContentFilteringService() override;

  static void AddInterface(service_manager::BinderRegistry* registry);

  // mojom::ContentFilteringService methods:
  void Log(const std::string& message) override;
  void GetRulesProvider(
      mojo::PendingReceiver<mojom::ContentFilterRulesProvider> receiver)
          override;
  void OnContentFiltered(
      int32_t render_frame_id, mojom::ContentFilterActionPtr action) override;
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTERING_SERVICE_H_
