// Copyright 2022 Neeva. All rights reserved.

#include "components/neeva/browser/content_filter_rules_config.h"

#include <algorithm>

#include "base/android/apk_assets.h"
#include "base/callback.h"
#include "base/files/file.h"
#include "base/memory/ptr_util.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "content/public/browser/browser_context.h"

namespace neeva {

// static
const char* ContentFilterRulesConfig::kValidRulesFiles[2] = {
  "easyprivacy",
  "easylist"
};

// static
const int ContentFilterRulesConfig::kUserDataKey;

// static
bool ContentFilterRulesConfig::IsValidRulesFile(const std::string& file) {
  for (const auto* valid_file : kValidRulesFiles) {
    if (file == valid_file)
      return true;
  }
  return false;
}

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
     // Ties the lifecycle of a RulesConfig with 1 browser context (aka. like a profile - cookies, etc.)
    browser_context->SetUserData(&kUserDataKey, base::WrapUnique(config));
  }
  return config;
}

void ContentFilterRulesConfig::EnableRulesFile(const std::string& file) {
  if (!IsValidRulesFile(file)) {
    LOG(ERROR) << "Unexpected rules file: " << file;
    return;
  }
  rules_files_enabled_.insert(file);
  ConfigChanged();
}

void ContentFilterRulesConfig::DisableAllRulesFiles() {
  rules_files_enabled_.clear();
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

void ContentFilterRulesConfig::ClearAllHostExclusions() {
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

// Invoked in content_filtering_service.cc
void ContentFilterRulesConfig::AddListener(
    mojo::PendingRemote<mojom::ContentFilterRulesListener> remote) {
  auto listener_id = listeners_.Add(std::move(remote));

  listeners_.Get(listener_id)->OnReceiveNewRules(GetRules());
}

ContentFilterRulesConfig::ContentFilterRulesConfig() = default;

void ContentFilterRulesConfig::ConfigChanged() {
  if (is_notify_pending_) {
    return;
  }
  is_notify_pending_ = true;

  // Notify callbacks asynchronously in case other ConfigChanged calls come in
  // immediately following this one. That way they all get batched up together
  // into a single update.
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::BindOnce(&ContentFilterRulesConfig::NotifyListeners,
                     weak_factory_.GetWeakPtr()));
}

void ContentFilterRulesConfig::NotifyListeners() {
  is_notify_pending_ = false;

  for (auto& listener : listeners_) {
    listener->OnReceiveNewRules(GetRules());
  }
}

mojom::ContentFilterRulesPtr ContentFilterRulesConfig::GetRules() const {
  if (!is_filtering_enabled_)
    return nullptr;

  auto rules = mojom::ContentFilterRules::New();
  rules->mode = mode_;

  std::vector<std::string> hosts(host_exclusions_.size());
  std::copy(host_exclusions_.begin(), host_exclusions_.end(), hosts.begin());
  rules->top_level_host_exclusions = std::move(hosts);

  for (const auto& file : rules_files_enabled_) {
    auto data = mojom::ContentFilterData::New();

    std::string apk_path = "assets/" + file + ".dat";

    base::MemoryMappedFile::Region region;
    base::ScopedFD fd(base::android::OpenApkAsset(apk_path, &region));
    if (fd == -1) {
      LOG(ERROR) << "Unable to open resource: " << apk_path;
      continue;
    }

    data->rules_name = file;
    data->rules_data_fd = mojo::PlatformHandle(std::move(fd));
    data->rules_data_offset = region.offset;
    data->rules_data_size = region.size;

    rules->data.push_back(std::move(data));
  }

  return rules;
}

}  // namespace neeva
