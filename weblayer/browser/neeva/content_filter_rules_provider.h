// Copyright 2022 Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_

#include "base/memory/weak_ptr.h"
#include "weblayer/browser/neeva/content_filter_rules_config.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace weblayer {
namespace neeva {

class ContentFilterRulesProvider : public mojom::ContentFilterRulesProvider {
 public:
  explicit ContentFilterRulesProvider(int render_process_id);
  ~ContentFilterRulesProvider() override;

  // mojom::ContentFilterRulesProvider methods:
  void RefreshRules(
      int64_t current_generation_num, RefreshRulesCallback callback) override;

 private:
  void SendRulesToClient(RefreshRulesCallback callback) const;

  base::WeakPtr<ContentFilterRulesConfig> config_;

  base::WeakPtrFactory<ContentFilterRulesProvider> weak_factory_{this};
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_
