// Copyright Neeva. All rights reserved.

#include "components/neeva/renderer/css_rule_list_matcher.h"

#include "components/neeva/flat/content_filter_rules_generated.h"

namespace neeva {

CssRuleListMatcher::CssRuleListMatcher(const flat::CssRuleList* rule_list)
    : rule_list_(rule_list) {
  // TODO: Build hashmap to optimize |domain_specific_selectors| lookup.
}

CssRuleListMatcher::~CssRuleListMatcher() = default;

std::string CssRuleListMatcher::GetStyleSheetForDomain(
    const base::StringPiece& domain) {
  // TODO: Add support for domain specific selectors.
  // TODO: Add caching? (maybe better done w/ WebString?)

  std::string stylesheet;
  stylesheet.reserve(4096);  // TODO: Choose more wisely?

  if (rule_list_->generic_selectors()) {
    for (const auto* selector : *rule_list_->generic_selectors()) {
      stylesheet.append(selector->str());
      stylesheet.append(1, ',');
    }
  }

  if (!stylesheet.empty()) {
    stylesheet.erase(stylesheet.end() - 1, stylesheet.end());
    stylesheet.append("{display:none!important;}");
  }

  return stylesheet;
}

}  // namespace neeva
