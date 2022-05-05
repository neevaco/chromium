// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_BROWSER_CONTENT_FILTERING_SERVICE_H_
#define COMPONENTS_NEEVA_BROWSER_CONTENT_FILTERING_SERVICE_H_

#include "components/neeva/common/content_filtering_service.mojom.h"
#include "services/service_manager/public/cpp/binder_registry.h"

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

#endif  // COMPONENTS_NEEVA_BROWSER_CONTENT_FILTERING_SERVICE_H_
