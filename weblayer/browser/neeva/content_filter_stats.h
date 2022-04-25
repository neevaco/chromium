// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_STATS_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_STATS_H_

#include <map>

#include "content/public/browser/document_user_data.h"

namespace weblayer {
namespace neeva {

class ContentFilterStats : public content::DocumentUserData<ContentFilterStats> {
 public:
  ~ContentFilterStats() override;

  void RecordFilteredHost(const std::string& host) {
    hosts_to_counts_[host] += 1;
  }

  const std::map<std::string, int>& data() const {
      return hosts_to_counts_; }

 private:
  explicit ContentFilterStats(content::RenderFrameHost* rfh);

  friend DocumentUserData;
  DOCUMENT_USER_DATA_KEY_DECL();

  std::map<std::string, int> hosts_to_counts_;
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_STATS_H_
