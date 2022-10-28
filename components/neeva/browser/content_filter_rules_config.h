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
// configuration. This is the controller the Browser process can use 
// to adjust filter rules in the Renderer process. 
class ContentFilterRulesConfig : public base::SupportsUserData::Data {
 public:
  static const char* kValidRulesFiles[2];
  static bool IsValidRulesFile(const std::string& rules_file);

  ~ContentFilterRulesConfig() override;

  static ContentFilterRulesConfig* Get(
      content::BrowserContext* browser_context);
  static ContentFilterRulesConfig* GetOrCreate(
      content::BrowserContext* browser_context);

  // Enable a specific rules file. These are string names corresponding to the
  // resource files without the file path or extension (e.g., "easylist" or
  // "easyprivacy").
  void EnableRulesFile(const std::string& rules_file);

  void DisableAllRulesFiles();
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

  std::set<std::string> rules_files_enabled_;
  std::set<std::string> host_exclusions_;
  mojom::ContentFilterMode mode_ = mojom::ContentFilterMode::BLOCK_COOKIES;
  bool is_filtering_enabled_ = false;
  bool is_notify_pending_ = false;

  mojo::RemoteSet<mojom::ContentFilterRulesListener> listeners_;

  base::WeakPtrFactory<ContentFilterRulesConfig> weak_factory_{this};
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_RULES_CONFIG_H_
