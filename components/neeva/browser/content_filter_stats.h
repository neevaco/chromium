// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_STATS_H_
#define COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_STATS_H_

#include <map>

#include "content/public/browser/document_user_data.h"

namespace neeva {

class ContentFilterStats : public content::DocumentUserData<ContentFilterStats> {
 public:
  using HostStats = std::map<std::string /*hostname*/, int /*count*/>;

  ~ContentFilterStats() override;

  void RecordFilteredHost(const std::string& rules_file, const std::string& host);

  void GetHostsForFilter(
      const std::string& rules_file, std::vector<std::string>* hosts) const;
  int GetHostCountsForFilter(
      const std::string& rules_file, const std::string& host) const;

 private:
  explicit ContentFilterStats(content::RenderFrameHost* rfh);

  friend DocumentUserData;
  DOCUMENT_USER_DATA_KEY_DECL();

  std::map<std::string /*rules_file*/, HostStats> data_;
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_STATS_H_
