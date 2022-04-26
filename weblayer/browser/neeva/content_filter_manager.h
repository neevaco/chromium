// Copyright 2022 Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_MANAGER_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_MANAGER_H_

#include <set>
#include <string>

#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/supports_user_data.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace content {
class BrowserContext;
}

namespace weblayer {
namespace neeva {

// Stored on each content::BrowserContext and holds the current rules
// configuration.
class ContentFilterManager : public base::SupportsUserData::Data {
 public:
  ~ContentFilterManager() override;

  static ContentFilterManager* Get(
      content::BrowserContext* browser_context);
  static ContentFilterManager* GetOrCreate(
      content::BrowserContext* browser_context);

  void SetRulesFile(const base::FilePath& rules_file);
  void SetMode(mojom::ContentFilterMode mode);
  void AddHostExclusion(const std::string& host);
  void RemoveHostExclusion(const std::string& host);
  void ClearAllHostExclusions(const std::string& host);
  void StartFiltering();
  void StopFiltering();

  bool is_filtering_enabled() const { return is_filtering_enabled_; }

  // Builds a snapshot of the current configuration. Runs asynchronously
  // due to file processing required.
  void Snapshot(
      base::OnceCallback<void(mojom::ContentFilterRulesPtr)> callback);

  class Observer : public base::CheckedObserver {
   public:
    virtual void OnChanged() = 0;
  };

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Enable clients to get a WeakPtr to instances. This allows observers to
  // safely hold a reference to the config.
  base::WeakPtr<ContentFilterManager> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 private:
  static const int kUserDataKey = 0;

  ContentFilterManager();
  void ConfigChanged();

  base::FilePath rules_file_;
  mojom::ContentFilterMode mode_ = mojom::ContentFilterMode::BLOCK_COOKIES;
  std::set<std::string> host_exclusions_;
  bool is_filtering_enabled_ = false;

  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<ContentFilterManager> weak_factory_{this};
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_MANAGER_H_
