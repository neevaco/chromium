// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_

#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace weblayer {
namespace neeva {

class ContentFilterRulesProvider : public mojom::ContentFilterRulesProvider {
 public:
  ContentFilterRulesProvider();
  ~ContentFilterRulesProvider() override;

  // mojom::ContentFilterRulesProvider methods:
  void RefreshRules(
      int64_t current_sequence_num, RefreshRulesCallback callback) override;
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_PROVIDER_H_
