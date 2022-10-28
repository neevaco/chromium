// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_CLIENT_H_
#define COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_CLIENT_H_

#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/supports_user_data.h"

namespace content {
class WebContents;
}

namespace neeva {

// Allows for associating a callback with a WebContents that will be notified
// whenever the ContentFilterStats for the main frame gets updated.
class ContentFilterClient : public base::SupportsUserData::Data {
 public:
  ~ContentFilterClient() override;

  static ContentFilterClient* Get(content::WebContents* web_contents);
  static ContentFilterClient* GetOrCreate(content::WebContents* web_contents);

  void set_callback(base::RepeatingClosure callback) { callback_ = callback; }
  void Notify();

 private:
  static const int kUserDataKey = 0;

  ContentFilterClient();
  void RunCallback();

  base::RepeatingClosure callback_;
  bool callback_pending_ = false;

  base::WeakPtrFactory<ContentFilterClient> weak_factory_{this};
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_CLIENT_H_
