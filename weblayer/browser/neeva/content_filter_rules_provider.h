// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_

#include "weblayer/browser/neeva/content_filter_rules_config.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace weblayer {
namespace neeva {

class ContentFilterRulesProvider : public mojom::ContentFilterRulesProvider,
                                   public ContentFilterRulesConfig::Observer {
 public:
  explicit ContentFilterRulesProvider(int render_process_id);
  ~ContentFilterRulesProvider() override;

  // mojom::ContentFilterRulesProvider methods:
  void RefreshRules(
      int64_t current_generation_num, RefreshRulesCallback callback) override;

  // ContentFilterRulesConfig::Observer:
  void OnContentFilterRulesConfigChanged() override;

 private:
  RefreshRulesCallback refresh_rules_callback_;
  base::WeakPtr<ContentFilterRulesConfig> config_;
  int64_t generation_num_ = 1;
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_
