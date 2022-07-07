// Copyright Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_RENDERER_CONTENT_FILTERING_AGENT_H__
#define COMPONENTS_NEEVA_RENDERER_CONTENT_FILTERING_AGENT_H__

#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "components/neeva/common/content_filtering_service.mojom.h"
#include "components/url_pattern_index/proto/rules.pb.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace base {
class MemoryMappedFile;
}

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

namespace neeva {

struct ContentFilteringAgentDeleter;

enum class ContentFilteringPolicy {
  kAllow,
  kBlockCookies,
  kBlockRequest
};

class ContentFilteringAgent
    : public mojom::ContentFilterRulesListener,
      public base::RefCountedThreadSafe<ContentFilteringAgent,
                                        ContentFilteringAgentDeleter> {
 public:
  explicit ContentFilteringAgent(
      blink::ThreadSafeBrowserInterfaceBrokerProxy* broker);

  std::unique_ptr<blink::URLLoaderThrottle> CreateThrottle(
      int render_frame_id, const blink::WebURLRequest& request);

  // The following methods may be called on a background thread.
  void OnContentFiltered(
      int32_t render_frame_id, mojom::ContentFilterActionPtr action);
  ContentFilteringPolicy GetPolicyForRequest(
      const GURL& url, const url::Origin& first_party_origin,
      url_pattern_index::proto::ElementType element_type) const;

  // mojom::ContentFilterRulesListener methods:
  void OnReceiveNewRules(mojom::ContentFilterRulesPtr new_rules) override;

 private:
  friend struct ContentFilteringAgentDeleter;

  ~ContentFilteringAgent() override;
  void DeleteOnCorrectThread() const;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  mojo::Remote<mojom::ContentFilteringService> service_;

  mojo::Receiver<mojom::ContentFilterRulesListener> receiver_{this};

  // Acquire |rules_lock_| before accessing any of the following fields.
  mutable base::Lock rules_lock_;
  mojom::ContentFilterRulesPtr rules_;
  std::unique_ptr<base::MemoryMappedFile> rules_data_;
  std::unique_ptr<url_pattern_index::UrlPatternIndexMatcher> matcher_;
};

struct ContentFilteringAgentDeleter {
  static void Destruct(const ContentFilteringAgent* agent) {
    agent->DeleteOnCorrectThread();
  }
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_RENDERER_CONTENT_FILTERING_AGENT_H__
