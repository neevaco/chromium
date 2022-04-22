// Copyright Neeva. All rights reserved.

#ifndef WEBLAYER_RENDERER_NEEVA_CONTENT_FILTERING_AGENT_H__
#define WEBLAYER_RENDERER_NEEVA_CONTENT_FILTERING_AGENT_H__

#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "components/url_pattern_index/proto/rules.pb.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "weblayer/common/neeva/content_filtering_service.mojom.h"

namespace blink {
class ThreadSafeBrowserInterfaceBrokerProxy;
class URLLoaderThrottle;
class WebURLRequest;
}

namespace url {
class Origin;
}

namespace url_pattern_index {
class UrlPatternIndexMatcher;
}

class GURL;

namespace weblayer {
namespace neeva {

struct ContentFilteringAgentDeleter;

enum class ContentFilteringPolicy {
  kAllow,
  kBlockCookies,
  kBlockRequest
};

class ContentFilteringAgent
    : public base::RefCountedThreadSafe<ContentFilteringAgent,
                                       ContentFilteringAgentDeleter> {
 public:
  explicit ContentFilteringAgent(
      blink::ThreadSafeBrowserInterfaceBrokerProxy* broker);

  std::unique_ptr<blink::URLLoaderThrottle> CreateThrottle(
      int render_frame_id, const blink::WebURLRequest& request);

  // The following methods may be called on a background thread.
  void Log(const std::string& message);
  void OnContentFiltered(
      int32_t render_frame_id, mojom::ContentFilterActionPtr action);
  ContentFilteringPolicy GetPolicyForRequest(
      const GURL& url, const url::Origin& first_party_origin,
      url_pattern_index::proto::ElementType element_type) const;

 private:
  friend struct ContentFilteringAgentDeleter;

  ~ContentFilteringAgent();
  void DeleteOnCorrectThread() const;
  void RefreshRules();
  void OnApplyNewRules(
      int64_t new_generation_num, mojom::ContentFilterRulesPtr new_rules);

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  mojo::Remote<mojom::ContentFilteringService> service_;
  int64_t current_generation_num_ = 0;

  // Acquire |rules_lock_| before accessing any of the following fields.
  mutable base::Lock rules_lock_;
  mojom::ContentFilterRulesPtr rules_;
  std::unique_ptr<url_pattern_index::UrlPatternIndexMatcher> matcher_;
  mojo::ScopedSharedBufferMapping rules_data_mapping_;
};

struct ContentFilteringAgentDeleter {
  static void Destruct(const ContentFilteringAgent* agent) {
    agent->DeleteOnCorrectThread();
  }
};

}  // namespace neeva
}  // namespace weblayer

#endif  // WEBLAYER_RENDERER_NEEVA_CONTENT_FILTERING_AGENT_H__
