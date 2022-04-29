// Copyright 2022 Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_CONFIG_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_CONFIG_H_

#include <set>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/supports_user_data.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/system/buffer.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace content {
class BrowserContext;
}

namespace weblayer {
namespace neeva {

// Stored on each content::BrowserContext and holds the current rules
// configuration.
class ContentFilterRulesConfig : public base::SupportsUserData::Data,
                                 public mojom::ContentFilterRulesProvider {
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

  int64_t rules_generation_num() const { return rules_generation_num_; }

  // Returns current rules. Return nullptr if there are no rules / if filtering
  // is disabled.
  mojom::ContentFilterRulesPtr GetRules() const;

  void AddReceiver(
      mojo::PendingReceiver<mojom::ContentFilterRulesProvider> receiver);

  // mojom::ContentFilterRulesProvider methods:
  void RefreshRules(
      int64_t current_generation_num, RefreshRulesCallback callback) override;

 private:
  static const int kUserDataKey = 0;

  ContentFilterRulesConfig();
  void ConfigChanged();
  void NotifyCallbacks();
  void SendRulesToClient(RefreshRulesCallback callback) const;

  //base::FilePath rules_file_;
  //mojo::ScopedSharedBufferHandle rules_file_buffer_;

  std::string rules_file_apk_path_;
  mojom::ContentFilterMode mode_ = mojom::ContentFilterMode::BLOCK_COOKIES;
  std::set<std::string> host_exclusions_;
  bool is_filtering_enabled_ = false;
  bool is_notify_pending_ = false;

  // This value is incremented each time the rules are updated. Initialized
  // to 0 to signify that rules_ are not generated yet.
  int64_t rules_generation_num_ = 0;

  mojom::ContentFilterRulesPtr rules_;
  std::vector<base::OnceClosure> refresh_rules_callbacks_;

  mojo::ReceiverSet<mojom::ContentFilterRulesProvider> receiver_set_;

  base::WeakPtrFactory<ContentFilterRulesConfig> weak_factory_{this};
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_CONFIG_H_
