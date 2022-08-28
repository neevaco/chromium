// Copyright Neeva. All rights reserved.

#include "components/neeva/renderer/css_rule_list_matcher.h"

#include "components/neeva/flat/content_filter_rules_generated.h"

namespace neeva {

CssRuleListMatcher::CssRuleListMatcher(const flat::CssRuleList* rule_list)
    : rule_list_(rule_list) {
}

CssRuleListMatcher::~CssRuleListMatcher() = default;

std::string CssRuleListMatcher::GetStyleSheetForDomain(
    const base::StringPiece& domain) {
  (void) rule_list_;
  return std::string();
}

}  // namespace neeva
