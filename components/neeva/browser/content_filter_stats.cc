// Copyright Neeva. All rights reserved.

#include "components/neeva/browser/content_filter_stats.h"

#include "components/neeva/browser/content_filter_rules_config.h"

namespace neeva {

ContentFilterStats::~ContentFilterStats() = default;

void ContentFilterStats::RecordFilteredHost(
    const std::string& rules_file, const std::string& host) {
  // TODO: Consider mapping a.foo.com to foo.com here instead of downstream.
  data_[rules_file][host] += 1;
}

void ContentFilterStats::GetHostsForFilter(
    const std::string& rules_file, std::vector<std::string>* hosts) const {
  if (rules_file != "*") {
    auto it = data_.find(rules_file);
    if (it != data_.end()) {
      for (const auto& entry : it->second) {
        hosts->push_back(entry.first);
      }
    }
  } else {
    for (const auto* filename : ContentFilterRulesConfig::kValidRulesFiles) {
      GetHostsForFilter(filename, hosts);
    }
  }
}

int ContentFilterStats::GetHostCountsForFilter(
    const std::string& rules_file, const std::string& host) const {
  int count = 0;
  if (rules_file != "*") {
    auto it = data_.find(rules_file);
    if (it != data_.end()) {
      auto result = it->second.find(host);
      if (result != it->second.end()) {
        count = result->second;
      }
    }
  } else {
    for (const auto* filename : ContentFilterRulesConfig::kValidRulesFiles) {
      count += GetHostCountsForFilter(filename, host);
    }
  }
  return count;
}

ContentFilterStats::ContentFilterStats(content::RenderFrameHost* rfh)
    : DocumentUserData(rfh) {}

DOCUMENT_USER_DATA_KEY_IMPL(ContentFilterStats);

}  // namespace neeva
