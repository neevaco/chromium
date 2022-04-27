// Copyright 2022 Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_CONFIG_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_CONFIG_H_

#include <queue>
#include <set>
#include <string>

#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/supports_user_data.h"
#include "mojo/public/cpp/system/buffer.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace content {
class BrowserContext;
}

namespace weblayer {
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

  void SetRulesFile(const base::FilePath& rules_file);
  void SetMode(mojom::ContentFilterMode mode);
  void AddHostExclusion(const std::string& host);
  void RemoveHostExclusion(const std::string& host);
  void ClearAllHostExclusions();
  void StartFiltering();
  void StopFiltering();

  bool is_filtering_enabled() const { return is_filtering_enabled_; }

  // Builds a snapshot of the current configuration. Runs asynchronously
  // due to file processing required.
  void Snapshot(
      base::OnceCallback<void(mojom::ContentFilterRulesPtr)> callback);

  class Observer : public base::CheckedObserver {
   public:
    virtual void OnContentFilterRulesConfigChanged() = 0;
  };

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Enable clients to get a WeakPtr to instances. This allows observers to
  // safely hold a reference to the config.
  base::WeakPtr<ContentFilterRulesConfig> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 private:
  static const int kUserDataKey = 0;

  ContentFilterRulesConfig();
  void ConfigChanged();
  void NotifyAllObservers();
  void CompleteSnapshot(
      base::OnceCallback<void(mojom::ContentFilterRulesPtr)> callback);
  void ReadRulesFile(base::OnceClosure continuation);
  void DoReadRulesFile();
  void DidReadRulesFile(
      base::FilePath rules_file_read, mojo::ScopedSharedBufferHandle buffer);

  bool is_reading_rules_file() const {
    return !read_rules_file_continuations_.empty();
  }

  base::FilePath rules_file_;
  mojo::ScopedSharedBufferHandle rules_file_buffer_;
  mojom::ContentFilterMode mode_ = mojom::ContentFilterMode::BLOCK_COOKIES;
  std::set<std::string> host_exclusions_;
  bool is_filtering_enabled_ = false;
  bool is_notify_pending_ = false;
  std::queue<base::OnceClosure> read_rules_file_continuations_;

  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<ContentFilterRulesConfig> weak_factory_{this};
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_CONFIG_H_
