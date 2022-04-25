// Copyright Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filter_stats.h"

namespace weblayer {
namespace neeva {

ContentFilterStats::~ContentFilterStats() = default;

ContentFilterStats::ContentFilterStats(content::RenderFrameHost* rfh)
    : DocumentUserData(rfh) {}

DOCUMENT_USER_DATA_KEY_IMPL(ContentFilterStats);

}  // namespace neeva
}  // namespace weblayer
