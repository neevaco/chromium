// Copyright 2022 Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filter_rules_config.h"

#include <algorithm>

#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "base/task/post_task.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "content/public/browser/browser_context.h"

namespace weblayer {
namespace neeva {

namespace {

mojo::ScopedSharedBufferHandle ReadFileToBuffer(
    const base::FilePath& file_path) {
  return mojo::ScopedSharedBufferHandle();  // XXX
}

}  // namespace

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
  rules_file_buffer_.reset();
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

void ContentFilterRulesConfig::Snapshot(
    base::OnceCallback<void(mojom::ContentFilterRulesPtr)> callback) {
  // Read the rules file into memory if needed.
  if (!rules_file_buffer_) {
    ReadRulesFile(
        base::BindOnce(&ContentFilterRulesConfig::Snapshot, GetWeakPtr(),
                       std::move(callback)));
    return;
  }

  std::vector<std::string> hosts(host_exclusions_.size());
  std::copy(host_exclusions_.begin(), host_exclusions_.end(), hosts.begin());

  auto rules = mojom::ContentFilterRules::New();
  rules->mode = mode_;
  rules->top_level_host_exclusions = std::move(hosts);
  rules->url_pattern_data = rules_file_buffer_->Clone(
      mojo::SharedBufferHandle::AccessMode::READ_ONLY);

  // Always dispatch the callback asynchronously so the caller is invoked
  // consistently.
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::BindOnce(std::move(callback), std::move(rules)));
}

void ContentFilterRulesConfig::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ContentFilterRulesConfig::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

ContentFilterRulesConfig::ContentFilterRulesConfig() = default;

void ContentFilterRulesConfig::ConfigChanged() {
  // Notify asynchronously so that multiple back-to-back calls to ConfigChanged
  // get coalesced into a single notification to observers.
  if (is_notify_pending_)
    return;
  is_notify_pending_ = true;
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::BindOnce(&ContentFilterRulesConfig::NotifyAllObservers, GetWeakPtr()));
}

void ContentFilterRulesConfig::NotifyAllObservers() {
  is_notify_pending_ = false;
  for (auto& observer : observers_) {
    observer.OnChanged();
  }
}

void ContentFilterRulesConfig::ReadRulesFile(base::OnceClosure continuation) {
  // There could be multiple overlapped calls to ReadRulesFile, and in that case
  // we just queue up the extra continuations. They will all get notified when
  // the file is read.
  bool needs_read = !is_reading_rules_file();
  read_rules_file_continuations_.push(std::move(continuation));
  if (!needs_read)
    return;
  DoReadRulesFile();
}

void ContentFilterRulesConfig::DoReadRulesFile() {
  base::PostTaskAndReplyWithResult(
      FROM_HERE,
      { base::MayBlock(), base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN },
      base::BindOnce(&ReadFileToBuffer, rules_file_),
      base::BindOnce(&ContentFilterRulesConfig::DidReadRulesFile, GetWeakPtr(),
                     rules_file_));
}

void ContentFilterRulesConfig::DidReadRulesFile(
    base::FilePath rules_file_read, mojo::ScopedSharedBufferHandle buffer) {
  // The value of rules_file_ may have changed while we were away. If so, then
  // start over and try again.
  if (rules_file_read != rules_file_) {
    DoReadRulesFile();
    return;
  }
  // No need to worry about re-entrancy during these callbacks given the
  // PostTask in Snapshot.
  while (!read_rules_file_continuations_.empty()) {
    std::move(read_rules_file_continuations_.front()).Run();
    read_rules_file_continuations_.pop();
  }
}

}  // namespace neeva
}  // namespace weblayer
