// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__
#define COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__

#include <string>
#include <unordered_map>

#include "base/containers/lru_cache.h"

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
  bool HasRules() const;

  const flat::DomainSpecificCssRule* FindRuleForHost(
      const std::string& host) const;

  // Must outlive this instance.
  const flat::CssRuleList* rule_list_;

  std::unordered_map<std::string /*domain*/,
                     const flat::DomainSpecificCssRule*> domain_map_;

  // A small cache of computed stylesheets. Sized to shoot for a total memory
  // usage of about 1.2M (based on a typical stylesheet size of 400K). This is
  // fixed overhead for each renderer, but since computing stylesheets can take
  // a few milliseconds, this cost is worth it.
  base::LRUCache<std::string /*host*/, std::string /*stylesheet*/> cache_{3};
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__
