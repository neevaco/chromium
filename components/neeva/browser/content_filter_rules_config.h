// Copyright 2022 Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_RULES_CONFIG_H_
#define COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_RULES_CONFIG_H_

#include <set>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/supports_user_data.h"
#include "components/neeva/common/content_filtering_service.mojom.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "mojo/public/cpp/system/buffer.h"

namespace content {
class BrowserContext;
}

namespace neeva {

// Stored on each content::BrowserContext and holds the current rules
// configuration.
class ContentFilterRulesConfig : public base::SupportsUserData::Data {
 public:
  ~ContentFilterRulesConfig() override;

  static ContentFilterRulesConfig* Get(
      content::BrowserContext* browser_context);
  static ContentFilterRulesConfig* GetOrCreate(
      content::BrowserContext* browser_context);

  void SetRulesFile(const std::string& rules_file_apk_path);
  void SetMode(mojom::ContentFilterMode mode);
  void AddHostExclusion(const std::string& host);
  void RemoveHostExclusion(const std::string& host);
  void ClearAllHostExclusions();
  void StartFiltering();
  void StopFiltering();

  void AddListener(
      mojo::PendingRemote<mojom::ContentFilterRulesListener> remote);

 private:
  static const int kUserDataKey = 0;

  ContentFilterRulesConfig();
  void ConfigChanged();
  void NotifyListeners();

  // Returns current rules. Return nullptr if there are no rules / if filtering
  // is disabled.
  mojom::ContentFilterRulesPtr GetRules() const;

  std::string rules_file_apk_path_;
  mojom::ContentFilterMode mode_ = mojom::ContentFilterMode::BLOCK_COOKIES;
  std::set<std::string> host_exclusions_;
  bool is_filtering_enabled_ = false;
  bool is_notify_pending_ = false;

  mojo::RemoteSet<mojom::ContentFilterRulesListener> listeners_;

  base::WeakPtrFactory<ContentFilterRulesConfig> weak_factory_{this};
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_RULES_CONFIG_H_
