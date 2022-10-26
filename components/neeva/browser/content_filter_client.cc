// Copyright Neeva. All rights reserved.

#include "components/neeva/browser/content_filter_client.h"

#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "content/public/browser/web_contents.h"

namespace neeva {

// static
const int ContentFilterClient::kUserDataKey;

ContentFilterClient::~ContentFilterClient() = default;

// static
ContentFilterClient* ContentFilterClient::Get(
    content::WebContents* web_contents) {
  return static_cast<ContentFilterClient*>(
      web_contents->GetUserData(&kUserDataKey));
}

// static
ContentFilterClient* ContentFilterClient::GetOrCreate(
    content::WebContents* web_contents) {
  auto* client = Get(web_contents);
  if (!client) {
    client = new ContentFilterClient(); 
    // client's lifecycle is now tied to the browser's web_contents.
    web_contents->SetUserData(&kUserDataKey, base::WrapUnique(client));
  }
  return client;
}

void ContentFilterClient::Notify() {
  // Run callback asynchronously and skip any notifications that come in while
  // we are waiting to run the callback. This helps avoid spammy notifications.

  if (!callback_ || callback_pending_)
    return;
  callback_pending_ = true;
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::BindOnce(&ContentFilterClient::RunCallback,
                     weak_factory_.GetWeakPtr()));
}

void ContentFilterClient::RunCallback() {
  callback_pending_ = false;
  if (callback_) {
    callback_.Run();
  }
}

ContentFilterClient::ContentFilterClient() = default;

}  // namespace neeva
