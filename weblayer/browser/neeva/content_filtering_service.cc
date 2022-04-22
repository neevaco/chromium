// Copyright Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filtering_service.h"

#include "base/logging.h"
#include "components/url_pattern_index/url_pattern_index.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "mojo/public/cpp/system/buffer.h"

namespace weblayer {
namespace neeva {

ContentFilteringService::ContentFilteringService() = default;

ContentFilteringService::~ContentFilteringService() = default;

// static
void ContentFilteringService::AddInterface(
    service_manager::BinderRegistry* registry) {
  auto create_service =
      [](mojo::PendingReceiver<mojom::ContentFilteringService> receiver) {
        mojo::MakeSelfOwnedReceiver(std::make_unique<ContentFilteringService>(),
                                    std::move(receiver));
      };
  registry->AddInterface(
      base::BindRepeating(create_service), content::GetUIThreadTaskRunner({}));
}

void ContentFilteringService::Log(const std::string& message) {
  LOG(ERROR) << ">>> " << message;
}

void ContentFilteringService::RefreshRules(
    int64_t current_sequence_num, RefreshRulesCallback callback) {
  // TODO: implement me!

  // XXX test out with some basic data... and validate that we are
  // getting the right outcome.

  if (current_sequence_num == 0) {
    flatbuffers::FlatBufferBuilder flat_builder;
    url_pattern_index::UrlPatternIndexBuilder index_builder(&flat_builder);

    // XXX add rules
    url_pattern_index::flat::UrlRuleBuilder rule_builder(flat_builder);
    rule_builder.add_url_pattern(flat_builder.CreateString(".doubleclick.net/"));
    rule_builder.add_element_types(url_pattern_index::proto::ELEMENT_TYPE_IMAGE);
    index_builder.IndexUrlRule(rule_builder.Finish());

    const auto index_offset = index_builder.Finish();
    flat_builder.Finish(index_offset);

    auto size = flat_builder.GetSize();
    auto* ptr = flat_builder.GetBufferPointer();

    auto shm = mojo::SharedBufferHandle::Create(size);
    {
      mojo::ScopedSharedBufferMapping mapping = shm->Map(size);
      memcpy(mapping.get(), ptr, size);
    }

    auto rules = mojom::ContentFilterRules::New();
    rules->mode = mojom::ContentFilterMode::BLOCK_REQUESTS;
    rules->url_pattern_data = std::move(shm);

    std::move(callback).Run(1, std::move(rules));
  }
}

void ContentFilteringService::OnContentFiltered(
    int32_t render_frame_id, mojom::ContentFilterActionPtr action) {
  // TODO: implement me!
  LOG(ERROR) << ">>> BLOCKED: " << action->host;
}

}  // namespace neeva
}  // namespace weblayer
