// Copyright 2022 Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filter_rules_config.h"

#include "base/memory/ptr_util.h"
#include "content/public/browser/browser_context.h"

namespace weblayer {
namespace neeva {

// static
const int ContentFilterRulesConfig::kUserDataKey;

ContentFilterRulesConfig::~ContentFilterRulesConfig() = default;

// static
ContentFilterRulesConfig* ContentFilterRulesConfig::Get(
    content::BrowserContext* browser_context) {
  return static_cast<ContentFilterRulesConfig*>(
      browser_context->GetUserData(&kUserDataKey));
}

// static
ContentFilterRulesConfig* ContentFilterRulesConfig::GetOrCreate(
    content::BrowserContext* browser_context) {
  auto* config = Get(browser_context);
  if (!config) {
    config = new ContentFilterRulesConfig();
    browser_context->SetUserData(&kUserDataKey, base::WrapUnique(config));
  }
  return config;
}

void ContentFilterRulesConfig::SetRulesFile(const base::FilePath& rules_file) {
  rules_file_ = rules_file;
  // TODO: read file into SHM instead. no need to keep the file path, right?
  ConfigChanged();
}

void ContentFilterRulesConfig::SetMode(mojom::ContentFilterMode mode) {
  mode_ = mode;
  ConfigChanged();
}

void ContentFilterRulesConfig::AddHostExclusion(const std::string& host) {
  host_exclusions_.insert(host);
  ConfigChanged();
}

void ContentFilterRulesConfig::RemoveHostExclusion(const std::string& host) {
  host_exclusions_.erase(host);
  ConfigChanged();
}

void ContentFilterRulesConfig::ClearAllHostExclusions(const std::string& host) {
  host_exclusions_.clear();
  ConfigChanged();
}

void ContentFilterRulesConfig::StartFiltering() {
  if (is_filtering_enabled_)
    return;
  is_filtering_enabled_ = true;
  ConfigChanged();
}

void ContentFilterRulesConfig::StopFiltering() {
  if (!is_filtering_enabled_)
    return;
  is_filtering_enabled_ = false;
  ConfigChanged();
}

mojom::ContentFilterRulesPtr ContentFilterRulesConfig::Snapshot() const {
  // TODO: clone SHM (readonly) and populate the ContentFilterRulesPtr.
  return mojom::ContentFilterRulesPtr();
}

void ContentFilterRulesConfig::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ContentFilterRulesConfig::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

ContentFilterRulesConfig::ContentFilterRulesConfig() = default;

void ContentFilterRulesConfig::ConfigChanged() {
  for (auto& observer : observers_) {
    observer.OnChanged();
  }
}

}  // namespace neeva
}  // namespace weblayer
