// Copyright Neeva. All rights reserved.

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
  void RefreshRules(
      int64_t current_sequence_num, RefreshRulesCallback callback) override;
  void OnContentFiltered(
      int32_t render_frame_id, mojom::ContentFilterActionPtr action) override;
};

}  // namespace neeva
}  // namespace weblayer
