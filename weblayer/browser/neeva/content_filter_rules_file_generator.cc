// Copyright 2022 Neeva. All rights reserved.

#include "weblayer/browser/neeva/content_filter_rules_file_generator.h"

#include <iostream>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/task/post_task.h"
#include "components/url_pattern_index/url_pattern_index.h"

using namespace url_pattern_index;

namespace weblayer {
namespace neeva {
namespace {

proto::ElementType ToElementType(std::string option, bool* negate) {
  if (base::StartsWith(option, "~")) {
    *negate = true;
    option.erase(0, 1);
  } else {
    *negate = false;
  }

  const struct {
    const char* name;
    proto::ElementType type;
  } kElementsMap[] = {
    { "other", proto::ELEMENT_TYPE_OTHER },
    { "script", proto::ELEMENT_TYPE_SCRIPT },
    { "image", proto::ELEMENT_TYPE_IMAGE },
    { "stylesheet", proto::ELEMENT_TYPE_STYLESHEET },
    { "object", proto::ELEMENT_TYPE_OBJECT },
    { "xmlhttprequest", proto::ELEMENT_TYPE_XMLHTTPREQUEST },
    { "subdocument", proto::ELEMENT_TYPE_SUBDOCUMENT },
    { "ping", proto::ELEMENT_TYPE_PING },
    { "media", proto::ELEMENT_TYPE_MEDIA },
    { "font", proto::ELEMENT_TYPE_FONT },
    { "popup", proto::ELEMENT_TYPE_POPUP },
    { "websocket", proto::ELEMENT_TYPE_WEBSOCKET },
    { "webtransport", proto::ELEMENT_TYPE_WEBTRANSPORT },
    { "webbundle", proto::ELEMENT_TYPE_WEBBUNDLE },
  };

  for (const auto& element : kElementsMap) {
    if (option == element.name)
      return element.type;
  }

  return proto::ELEMENT_TYPE_UNSPECIFIED;
}

class Generator {
 public:
  Generator() : index_builder_(&flat_builder_) {}

  // Rule definition is here:
  // https://help.eyeo.com/en/adblockplus/how-to-write-filters
  void BuildRule(const base::StringPiece& line) {
    if (line.empty() || line[0] == '!')
      return;

    // Drop CSS rules
    if (line.find("##") != base::StringPiece::npos)
      return;

    std::cout << std::endl << "line: " << line << std::endl;

    // Reverse search for first '$' as a regex might contain that character.
    std::string pattern, options;
    size_t divider_index = line.find_last_of("$");
    if (divider_index == base::StringPiece::npos) {
      pattern = std::string(line.begin(), line.end());
    } else {
      pattern = std::string(line.begin(), divider_index);
      options = std::string(line.begin() + divider_index + 1, line.end());
    }

    bool is_exception = false;
    if (base::StartsWith(pattern, "@@")) {
      is_exception = true;
      pattern.erase(0, 2);
    }

    proto::AnchorType left_anchor;
    if (base::StartsWith(pattern, "||")) {
      left_anchor = proto::ANCHOR_TYPE_SUBDOMAIN;
      pattern.erase(0, 2);
    } else if (base::StartsWith(pattern, "|")) {
      left_anchor = proto::ANCHOR_TYPE_BOUNDARY;
      pattern.erase(0, 1);
    } else {
      left_anchor = proto::ANCHOR_TYPE_NONE;
    }

    proto::AnchorType right_anchor;
    if (base::EndsWith(pattern, "|")) {
      right_anchor = proto::ANCHOR_TYPE_BOUNDARY;
      pattern.erase(pattern.size() - 1, 1);
    } else {
      right_anchor = proto::ANCHOR_TYPE_NONE;
    }

    proto::UrlPatternType pattern_type;
    if (base::StartsWith(pattern, "/") && base::EndsWith(pattern, "/")) {
      pattern_type = proto::URL_PATTERN_TYPE_REGEXP;
      pattern.erase(0, 1);
      pattern.erase(pattern.size() - 1, 1);
    } else if (pattern.find('*') != std::string::npos ||
               pattern.find('^') != std::string::npos) {
      pattern_type = proto::URL_PATTERN_TYPE_WILDCARDED;
    } else {
      pattern_type = proto::URL_PATTERN_TYPE_SUBSTRING;
    }

    std::vector<std::string> domains;
    proto::SourceType source_type = proto::SOURCE_TYPE_ANY;
    proto::ElementType element_types = proto::ELEMENT_TYPE_UNSPECIFIED;
    if (options.empty()) {
      element_types = proto::ELEMENT_TYPE_ALL;
    } else {
      const auto options_vector =
          base::SplitString(options, ",", base::KEEP_WHITESPACE,
                            base::SPLIT_WANT_NONEMPTY);
      for (std::string option : options_vector) {
        std::cout << "option: " << option << std::endl;

        // This assumes that if one element type is negated, then all will
        // be negated.
        bool negate;
        proto::ElementType type = ToElementType(option, &negate);
        if (type != proto::ELEMENT_TYPE_UNSPECIFIED) {
          if (negate) {
            if (element_types == proto::ELEMENT_TYPE_UNSPECIFIED)
              element_types = proto::ELEMENT_TYPE_ALL;
            element_types =
                static_cast<proto::ElementType>(element_types & ~type);
          } else {
            element_types =
                static_cast<proto::ElementType>(element_types | type);
          }
        } else if (option == "third-party") {
          source_type = proto::SOURCE_TYPE_THIRD_PARTY;
        } else if (option == "~third-party") {
          source_type = proto::SOURCE_TYPE_FIRST_PARTY;
        } else if (base::StartsWith(option, "domain=")) {
          option.erase(0, 7);
          domains =
              base::SplitString(option, "|", base::KEEP_WHITESPACE,
                                base::SPLIT_WANT_NONEMPTY);
          
        }
      }

    }

    std::cout << "pattern: " << pattern << std::endl;
    std::cout << "left_anchor: " << left_anchor << ", ";
    std::cout << "right_anchor: " << right_anchor << ", ";
    std::cout << "is_exception: " << is_exception << ", ";
    std::cout << "pattern_type: " << pattern_type << std::endl;
    std::cout << "type mask: " << std::hex << element_types << std::endl;

    std::cout << "domains: ";
    for (const auto& domain : domains)
      std::cout << domain << ", ";
    std::cout << std::endl;

    proto::UrlRule rule;

    rule.set_semantics(is_exception ? proto::RULE_SEMANTICS_ALLOWLIST :
                                      proto::RULE_SEMANTICS_BLOCKLIST);
    rule.set_source_type(source_type);
    rule.set_element_types(element_types);

    rule.set_url_pattern_type(pattern_type);
    rule.set_anchor_left(left_anchor);
    rule.set_anchor_right(right_anchor);
    rule.set_match_case(true);
    rule.set_url_pattern(pattern);

    AddInitiatorDomains(domains, &rule);

    AddUrlRule(rule);
  }
    
  void Finish() {
    flat_builder_.Finish(index_builder_.Finish());
  }

  uint32_t GetSize() const {
    return flat_builder_.GetSize();
  }

  uint8_t* GetBufferPointer() const {
    return flat_builder_.GetBufferPointer();
  }

 private:
  bool AddUrlRule(const proto::UrlRule& rule) {
    auto offset = SerializeUrlRule(rule, &flat_builder_, &domain_map_);
    if (offset.o) {
      index_builder_.IndexUrlRule(offset);
    } else {
      printf(">>> SerializeUrlRule failed!\n");
    }
    return !!offset.o;
  }

  void AddInitiatorDomains(const std::vector<std::string>& initiator_domains,
                           proto::UrlRule* rule) {
    for (std::string domain_pattern : initiator_domains) {
      DCHECK(!domain_pattern.empty());
      auto* domain = rule->add_initiator_domains();
      if (domain_pattern[0] == '~') {
        domain_pattern.erase(0, 1);
        domain->set_exclude(true);
      }
      domain->set_domain(std::move(domain_pattern));
    }
  }

  flatbuffers::FlatBufferBuilder flat_builder_;
  UrlPatternIndexBuilder index_builder_;
  FlatDomainMap domain_map_;
};

}  // namespace

// static
bool ContentFilterRulesFileGenerator::GenerateNow(
    const base::FilePath& input_file,
    const base::FilePath& output_file) {
  std::string input;
  if (!base::ReadFileToString(input_file, &input)) {
    LOG(ERROR) << ">>> Failed to read input file: " << input_file;
    return false;
  }

  printf(">>> input is %lu bytes in length\n", input.size());

  base::StringTokenizer line_tokenizer(input, "\n");
  // Make sure we support the version.
  if (!line_tokenizer.GetNext() ||
          line_tokenizer.token_piece() != "[Adblock Plus 1.1]") {
    LOG(ERROR) << ">>> Unsupported file format";
    return false;
  }

  Generator generator;

  while (line_tokenizer.GetNext()) {
    generator.BuildRule(line_tokenizer.token_piece());
  }

  generator.Finish();

  base::File output(
      output_file, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  char* bytes = reinterpret_cast<char*>(generator.GetBufferPointer());
  int bytes_to_write = static_cast<int>(generator.GetSize());
  if (output.WriteAtCurrentPos(bytes, bytes_to_write) != bytes_to_write) {
    LOG(ERROR) << ">>> Failed to write output file: " << output_file;
    return false;
  }

  return true;
}

// static
void ContentFilterRulesFileGenerator::Generate(
    const base::FilePath& input_file,
    const base::FilePath& output_file,
    base::OnceCallback<void(bool)> callback) {
  base::PostTaskAndReplyWithResult(
      FROM_HERE,
      { base::MayBlock(), base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN },
      base::BindOnce(&ContentFilterRulesFileGenerator::GenerateNow,
                     input_file, output_file),
      std::move(callback));
}

#if 0
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
#endif

}  // namespace neeva
}  // namespace weblayer
