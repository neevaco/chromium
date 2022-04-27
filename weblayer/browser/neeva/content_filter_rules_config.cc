// Copyright 2022 Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filter_rules_config.h"

#include <algorithm>

#include "base/callback.h"
#include "base/files/file.h"
#include "base/memory/ptr_util.h"
#include "base/task/post_task.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "content/public/browser/browser_context.h"

namespace weblayer {
namespace neeva {

namespace {

mojo::ScopedSharedBufferHandle ReadFileToBuffer(
    const base::FilePath& file_path) {
  // TODO: Figure out how to just mmap the file instead of copying it here!
  // Looks like we can use `mojo::WrapPlatformSharedMemoryRegion` for this.
  // We should be able to use PlatformSharedMemoryRegion::Take()

  base::File file(file_path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  int64_t size = file.GetLength();
  if (size < 0) {
    LOG(ERROR) << "Could not read file: " << file_path;
    return mojo::ScopedSharedBufferHandle();
  }

  auto buffer = mojo::SharedBufferHandle::Create(size);
  if (!buffer) {
    LOG(ERROR) << "Could not allocate buffer";
    return mojo::ScopedSharedBufferHandle();
  }

  auto mapping = buffer->Map(size);
  if (!mapping) {
    LOG(ERROR) << "Could not map buffer";
    return mojo::ScopedSharedBufferHandle();
  }

  if (file.ReadAtCurrentPos(static_cast<char*>(mapping.get()), size) == -1) {
    LOG(ERROR) << "Could not read file";
    return mojo::ScopedSharedBufferHandle();
  }

  return buffer;
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

mojom::ContentFilterRulesPtr ContentFilterRulesConfig::GetRules() const {
  mojom::ContentFilterRulesPtr result;
  if (is_filtering_enabled_ && rules_) {
    result = mojom::ContentFilterRules::New();
    result->mode = rules_->mode;
    result->top_level_host_exclusions = rules_->top_level_host_exclusions;
    if (rules_->url_pattern_data) {
      result->url_pattern_data = rules_->url_pattern_data->Clone(
          mojo::SharedBufferHandle::AccessMode::READ_ONLY);
    }
  }
  return result;
}

void ContentFilterRulesConfig::NotifyOnRulesUpdate(base::OnceClosure callback) {
  rules_update_callbacks_.push(std::move(callback));
}

ContentFilterRulesConfig::ContentFilterRulesConfig() = default;

void ContentFilterRulesConfig::ConfigChanged() {
  ++config_generation_num_;

  if (is_update_pending_) {
    return;
  }
  is_update_pending_ = true;

  // Start regenerating rules asynchronously in case other ConfigChanged calls
  // come in immediately following this one. That way they all get batched up
  // together into a single update. Pass along the generation number of the
  // config so we can see when we get done if the configuration has since
  // changed.
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::BindOnce(&ContentFilterRulesConfig::StartUpdate, GetWeakPtr()));
}

void ContentFilterRulesConfig::StartUpdate() {
  // Read the rules file into memory if needed.
  if (!rules_file_buffer_) {
    ReadRulesFile(
        base::BindOnce(&ContentFilterRulesConfig::FinishUpdate, GetWeakPtr()));
    return;
  }
  FinishUpdate();
}

void ContentFilterRulesConfig::FinishUpdate() {
  is_update_pending_ = false;
  rules_generation_num_ = config_generation_num_;

  // If the buffer is null at this point, then it means there was an error
  // trying to read the rules file. If the file was changed via SetRulesFile,
  // then we would have tried reading that new file. See DidReadRulesFile.

  if (rules_file_buffer_) {
    rules_ = mojom::ContentFilterRules::New();
    rules_->mode = mode_;

    std::vector<std::string> hosts(host_exclusions_.size());
    std::copy(host_exclusions_.begin(), host_exclusions_.end(), hosts.begin());
    rules_->top_level_host_exclusions = std::move(hosts);

    rules_->url_pattern_data = rules_file_buffer_->Clone(
        mojo::SharedBufferHandle::AccessMode::READ_ONLY);
  } else {
    rules_.reset();
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

  // Cache the memory buffer here.
  // TODO: Would be ideal to have a way to share this across multiple profiles,
  // especially with the incognito profile. We can build a separate cache for
  // these buffers to enable that. Using mmap to "read" the files would also
  // solve this.
  rules_file_buffer_ = std::move(buffer);

  // No need to worry about re-entrancy during these callbacks given the
  // PostTask in Snapshot.
  while (!read_rules_file_continuations_.empty()) {
    std::move(read_rules_file_continuations_.front()).Run();
    read_rules_file_continuations_.pop();
  }
}

}  // namespace neeva
}  // namespace weblayer
