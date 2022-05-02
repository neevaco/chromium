// Copyright Neeva. All rights reserved.

#include "components/neeva/browser/content_filter_stats.h"

namespace neeva {

ContentFilterStats::~ContentFilterStats() = default;

void ContentFilterStats::RecordFilteredHost(const std::string& host) {
  // TODO: Consider mapping a.foo.com to foo.com here instead of downstream.
  hosts_to_counts_[host] += 1;
}

ContentFilterStats::ContentFilterStats(content::RenderFrameHost* rfh)
    : DocumentUserData(rfh) {}

DOCUMENT_USER_DATA_KEY_IMPL(ContentFilterStats);

}  // namespace neeva
