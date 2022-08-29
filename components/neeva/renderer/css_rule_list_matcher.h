// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__
#define COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__

#include <string>
#include <unordered_map>

namespace neeva {

namespace flat {
struct CssRuleList;
struct DomainSpecificCssRule;
}  // namespace flat

class CssRuleListMatcher {
 public:
  explicit CssRuleListMatcher(const flat::CssRuleList* rule_list);
  ~CssRuleListMatcher();

 std::string GetStyleSheetForHost(const std::string& host);

 private:
  const flat::DomainSpecificCssRule* FindRuleForHost(
      const std::string& host) const;

  // Must outlive this instance.
  const flat::CssRuleList* rule_list_;

  std::unordered_map<const char* /*domain*/,
                     const flat::DomainSpecificCssRule*> domain_map_;
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__
