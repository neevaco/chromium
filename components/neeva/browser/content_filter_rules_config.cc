// Copyright 2022 Neeva. All rights reserved.

#include "components/neeva/browser/content_filter_rules_config.h"

#include <algorithm>

#include "base/android/apk_assets.h"
#include "base/callback.h"
#include "base/files/file.h"
#include "base/memory/ptr_util.h"
#include "base/task/post_task.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "content/public/browser/browser_context.h"

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

void ContentFilterRulesConfig::SetRulesFile(
    const std::string& rules_file_apk_path) {
  rules_file_apk_path_ = rules_file_apk_path;
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

void ContentFilterRulesConfig::AddReceiver(
    mojo::PendingReceiver<mojom::ContentFilterRulesProvider> receiver) {
  receiver_set_.Add(this, std::move(receiver));
}

void ContentFilterRulesConfig::RefreshRules(
    int64_t current_generation_num, RefreshRulesCallback callback) {
  if (rules_generation_num_ == current_generation_num) {
    refresh_rules_callbacks_.push_back(
        base::BindOnce(&ContentFilterRulesConfig::SendRulesToClient,
                       weak_factory_.GetWeakPtr(), std::move(callback)));
  } else {
    SendRulesToClient(std::move(callback));
  }
}

ContentFilterRulesConfig::ContentFilterRulesConfig() = default;

void ContentFilterRulesConfig::ConfigChanged() {
  ++rules_generation_num_;

  if (is_notify_pending_) {
    return;
  }
  is_notify_pending_ = true;

  // Notify callbacks asynchronously in case other ConfigChanged calls come in
  // immediately following this one. That way they all get batched up together
  // into a single update.
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::BindOnce(&ContentFilterRulesConfig::NotifyCallbacks,
                     weak_factory_.GetWeakPtr()));
}

void ContentFilterRulesConfig::NotifyCallbacks() {
  is_notify_pending_ = false;

  auto callbacks = std::move(refresh_rules_callbacks_);
  for (auto& callback : callbacks) {
    std::move(callback).Run();
  }
}

void ContentFilterRulesConfig::SendRulesToClient(
    RefreshRulesCallback callback) const {
  std::move(callback).Run(rules_generation_num_, GetRules());
}

mojom::ContentFilterRulesPtr ContentFilterRulesConfig::GetRules() const {
  if (!is_filtering_enabled_)
    return nullptr;

  base::MemoryMappedFile::Region region;
  base::ScopedFD fd(base::android::OpenApkAsset(rules_file_apk_path_, &region));
  if (fd == -1)
    return nullptr;

  auto rules = mojom::ContentFilterRules::New();
  rules->mode = mode_;

  std::vector<std::string> hosts(host_exclusions_.size());
  std::copy(host_exclusions_.begin(), host_exclusions_.end(), hosts.begin());
  rules->top_level_host_exclusions = std::move(hosts);

  rules->rules_data_fd = mojo::PlatformHandle(std::move(fd));
  rules->rules_data_offset = region.offset;
  rules->rules_data_size = region.size;

  return rules;
}

}  // namespace neeva
