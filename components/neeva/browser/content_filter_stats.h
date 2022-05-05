// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_STATS_H_
#define COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_STATS_H_

#include <map>

#include "content/public/browser/document_user_data.h"

namespace neeva {

class ContentFilterStats : public content::DocumentUserData<ContentFilterStats> {
 public:
  ~ContentFilterStats() override;

  void RecordFilteredHost(const std::string& host);

  const std::map<std::string, int>& data() const {
      return hosts_to_counts_; }

 private:
  explicit ContentFilterStats(content::RenderFrameHost* rfh);

  friend DocumentUserData;
  DOCUMENT_USER_DATA_KEY_DECL();

  std::map<std::string, int> hosts_to_counts_;
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_STATS_H_
