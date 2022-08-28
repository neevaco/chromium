// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__
#define COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__

#include <string>

#include "base/strings/string_piece.h"

namespace neeva {

namespace flat {
struct CssRuleList;
}  // namespace flat

class CssRuleListMatcher {
 public:
  explicit CssRuleListMatcher(const flat::CssRuleList* rule_list);
  ~CssRuleListMatcher();

 std::string GetStyleSheetForDomain(const base::StringPiece& domain);

 private:
  // Must outlive this instance.
  const flat::CssRuleList* rule_list_;
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_RENDERER_CSS_RULE_LIST_MATCHER_H__
