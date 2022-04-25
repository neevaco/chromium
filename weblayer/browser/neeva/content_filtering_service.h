// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTERING_SERVICE_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTERING_SERVICE_H_

#include "services/service_manager/public/cpp/binder_registry.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace weblayer {
namespace neeva {

class ContentFilteringService : public mojom::ContentFilteringService {
 public:
  explicit ContentFilteringService(int render_process_id);
  ~ContentFilteringService() override;

  static void AddInterface(
      service_manager::BinderRegistry* registry, int render_process_id);

  // mojom::ContentFilteringService methods:
  void Log(const std::string& message) override;
  void GetRulesProvider(
      mojo::PendingReceiver<mojom::ContentFilterRulesProvider> receiver)
          override;
  void OnContentFiltered(
      int32_t render_frame_id, mojom::ContentFilterActionPtr action) override;

 private:
  int render_process_id_;
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTERING_SERVICE_H_
