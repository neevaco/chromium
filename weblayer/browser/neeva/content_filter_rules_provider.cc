// Copyright Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filter_rules_provider.h"

#include "base/logging.h"
#include "components/url_pattern_index/url_pattern_index.h"
#include "mojo/public/cpp/system/buffer.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace weblayer {
namespace neeva {

ContentFilterRulesProvider::ContentFilterRulesProvider() = default;

ContentFilterRulesProvider::~ContentFilterRulesProvider() = default;

void ContentFilterRulesProvider::RefreshRules(
    int64_t current_sequence_num, RefreshRulesCallback callback) {
  LOG(ERROR) << ">>> RefreshRules()";

  // TODO: implement me!

  // XXX test out with some basic data... and validate that we are
  // getting the right outcome.

  if (current_sequence_num == 0) {
    flatbuffers::FlatBufferBuilder flat_builder;
    url_pattern_index::UrlPatternIndexBuilder index_builder(&flat_builder);

    // XXX add rules
    url_pattern_index::flat::UrlRuleBuilder rule_builder(flat_builder);
    rule_builder.add_options(
        url_pattern_index::flat::OptionFlag_APPLIES_TO_FIRST_PARTY |
        url_pattern_index::flat::OptionFlag_APPLIES_TO_THIRD_PARTY);
    rule_builder.add_url_pattern(flat_builder.CreateString(".doubleclick.net/"));
    rule_builder.add_element_types(url_pattern_index::proto::ELEMENT_TYPE_IMAGE);
    index_builder.IndexUrlRule(rule_builder.Finish());

    const auto index_offset = index_builder.Finish();
    flat_builder.Finish(index_offset);

    auto size = flat_builder.GetSize();
    auto* ptr = flat_builder.GetBufferPointer();

    // TEST!!!
    {
      // [https://googleads.g.doubleclick.net/pagead/viewthroughconversion/986255830/?value=0&guid=ON&script=0] dest=8

      const char* test_url_string =
          "https://googleads.g.doubleclick.net/pagead/viewthroughconversion/986255830/?value=0&guid=ON&script=0";

      const auto* flat_index = url_pattern_index::flat::GetUrlPatternIndex(ptr);
      auto matcher = std::make_unique<url_pattern_index::UrlPatternIndexMatcher>(flat_index);
      
      auto* match = matcher->FindMatch(
          GURL(test_url_string),
          url::Origin::Create(GURL("https://cnn.com/")),
          url_pattern_index::proto::ELEMENT_TYPE_IMAGE,
          url_pattern_index::proto::ACTIVATION_TYPE_UNSPECIFIED,
          true,
          false,
          url_pattern_index::UrlPatternIndexMatcher::EmbedderConditionsMatcher(),
          url_pattern_index::UrlPatternIndexMatcher::FindRuleStrategy::kAny);
      LOG(ERROR) << ">>> TEST " << (match ? "found match" : "no match");
    }

    auto shm = mojo::SharedBufferHandle::Create(size);
    {
      mojo::ScopedSharedBufferMapping mapping = shm->Map(size);
      memcpy(mapping.get(), ptr, size);
    }

    auto rules = mojom::ContentFilterRules::New();
    rules->mode = mojom::ContentFilterMode::BLOCK_REQUESTS;
    rules->url_pattern_data = std::move(shm);

    LOG(ERROR) << ">>> sending rules...";
    std::move(callback).Run(1, std::move(rules));
  }
}

}  // namespace neeva
}  // namespace weblayer
