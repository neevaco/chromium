// Copyright 2022 Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filter_manager.h"

#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/browser_context.h"

namespace weblayer {
namespace neeva {

// static
const int ContentFilterManager::kUserDataKey;

ContentFilterManager::~ContentFilterManager() = default;

// static
ContentFilterManager* ContentFilterManager::Get(
    content::BrowserContext* browser_context) {
  return static_cast<ContentFilterManager*>(
      browser_context->GetUserData(&kUserDataKey));
}

// static
ContentFilterManager* ContentFilterManager::GetOrCreate(
    content::BrowserContext* browser_context) {
  auto* config = Get(browser_context);
  if (!config) {
    config = new ContentFilterManager();
    browser_context->SetUserData(&kUserDataKey, base::WrapUnique(config));
  }
  return config;
}

void ContentFilterManager::SetRulesFile(const base::FilePath& rules_file) {
  rules_file_ = rules_file;
  // TODO: invalidate existing SHM.
  ConfigChanged();
}

void ContentFilterManager::SetMode(mojom::ContentFilterMode mode) {
  mode_ = mode;
  ConfigChanged();
}

void ContentFilterManager::AddHostExclusion(const std::string& host) {
  host_exclusions_.insert(host);
  ConfigChanged();
}

void ContentFilterManager::RemoveHostExclusion(const std::string& host) {
  host_exclusions_.erase(host);
  ConfigChanged();
}

void ContentFilterManager::ClearAllHostExclusions(const std::string& host) {
  host_exclusions_.clear();
  ConfigChanged();
}

void ContentFilterManager::StartFiltering() {
  if (is_filtering_enabled_)
    return;
  is_filtering_enabled_ = true;
  ConfigChanged();
}

void ContentFilterManager::StopFiltering() {
  if (!is_filtering_enabled_)
    return;
  is_filtering_enabled_ = false;
  ConfigChanged();
}

void ContentFilterManager::Snapshot(
    base::OnceCallback<void(mojom::ContentFilterRulesPtr)> callback) {
  // TODO: read rules file into SHM, clone SHM (readonly) and populate the
  // ContentFilterRulesPtr.
}

void ContentFilterManager::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ContentFilterManager::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

ContentFilterManager::ContentFilterManager() = default;

void ContentFilterManager::ConfigChanged() {
  for (auto& observer : observers_) {
    observer.OnChanged();
  }
}

}  // namespace neeva
}  // namespace weblayer
