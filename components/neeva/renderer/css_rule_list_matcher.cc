// Copyright Neeva. All rights reserved.

#include "components/neeva/renderer/css_rule_list_matcher.h"

#include <algorithm>

#include "base/logging.h"
#include "components/neeva/flat/content_filter_rules_generated.h"

namespace neeva {
namespace {

void AppendSelector(
    const flatbuffers::String* selector, std::string* selectors) {
  selectors->append(selector->begin(), selector->end());
  selectors->append(1, ',');
}

bool DomainAllowsSelector(const flat::DomainSpecificCssRule* rule,
                          const flatbuffers::String* selector) {
  for (const auto* excluded : *rule->excluded_selectors()) {
    if (std::equal(selector->begin(), selector->end(),
                   excluded->begin(), excluded->end())) {
      return false;
    }
  }
  return true;
}

}  // namespace

CssRuleListMatcher::CssRuleListMatcher(const flat::CssRuleList* rule_list)
    : rule_list_(rule_list) {
  // Build hashmap to optimize |domain_specific_selectors| lookup.
  if (rule_list_->domain_specific_selectors()) {
    for (const auto* rule : *rule_list_->domain_specific_selectors()) {
      if (!rule->domain())
        continue;
      domain_map_.insert(std::make_pair(rule->domain()->c_str(), rule));
    }
  }
}

CssRuleListMatcher::~CssRuleListMatcher() = default;

std::string CssRuleListMatcher::GetStyleSheetForHost(
    const std::string& host) {
  // TODO: Add caching? (maybe better done w/ WebString?)

  std::string stylesheet;
  stylesheet.reserve(4096);  // TODO: Choose more wisely?

  const flat::DomainSpecificCssRule* domain_rule = FindRuleForHost(host);

  if (rule_list_->generic_selectors()) {
    for (const auto* selector : *rule_list_->generic_selectors()) {
      if (domain_rule && !DomainAllowsSelector(domain_rule, selector))
        continue;
      AppendSelector(selector, &stylesheet);
    }
  }

  // Add domain specific selectors.
  if (domain_rule) {
    for (const auto* selector : *domain_rule->included_selectors())
      AppendSelector(selector, &stylesheet);
  }

  if (!stylesheet.empty()) {
    stylesheet.erase(stylesheet.end() - 1, stylesheet.end());
    stylesheet.append("{display:none!important;}");
  }

  return stylesheet;
}

const flat::DomainSpecificCssRule* CssRuleListMatcher::FindRuleForHost(
    const std::string& host) const {
  // Check given |host| first. If not found, look for matching subdomain.

  auto it = domain_map_.find(host.c_str());
  if (it != domain_map_.end())
    return it->second;

  auto dot_offset = host.find_first_of('.');
  if (dot_offset == std::string::npos)
    return nullptr;

  // Require at least one dot in the subdomain.
  std::string subdomain = host.substr(dot_offset + 1);
  if (subdomain.find_first_of('.') == std::string::npos)
    return nullptr;

  return FindRuleForHost(subdomain);
}

}  // namespace neeva
