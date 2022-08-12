// Copyright 2022 Neeva. All rights reserved.

#include "components/neeva/tools/content_filter_rules_file_generator.h"

#include <iostream>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "components/url_pattern_index/url_pattern_index.h"

using namespace url_pattern_index;

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

    std::cout << "Processing: " << line << std::endl;

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
      // Note: Regexps are not yet supported by UrlPatternIndexBuilder, but
      // we parse them anyways.
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
    proto::ElementType element_types = proto::ELEMENT_TYPE_ALL;
    if (!options.empty()) {
      const auto options_vector =
          base::SplitString(options, ",", base::KEEP_WHITESPACE,
                            base::SPLIT_WANT_NONEMPTY);
      for (std::string option : options_vector) {
        //std::cout << "option: " << option << std::endl;

        // This assumes that if one element type is negated, then all will
        // be negated.
        bool negate;
        proto::ElementType type = ToElementType(option, &negate);
        if (type != proto::ELEMENT_TYPE_UNSPECIFIED) {
          if (negate) {
            element_types =
                static_cast<proto::ElementType>(element_types & ~type);
          } else {
            if (element_types == proto::ELEMENT_TYPE_ALL) {
              element_types = type;
            } else {
              element_types =
                  static_cast<proto::ElementType>(element_types | type);
            }
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

    /* Uncomment to help debug.
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
    */

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
      std::cerr << "ERROR: SerializeUrlRule failed!" << std::endl;
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
bool ContentFilterRulesFileGenerator::Generate(
    const base::FilePath& input_file,
    const base::FilePath& output_file) {
  std::string input;
  if (!base::ReadFileToString(input_file, &input)) {
    std::cerr << "ERROR: Failed to read input file: " << input_file << std::endl;
    return false;
  }

  base::StringTokenizer line_tokenizer(input, "\n");
  // Make sure we support the version.
  if (!line_tokenizer.GetNext() ||
          line_tokenizer.token_piece() != "[Adblock Plus 1.1]") {
    std::cerr << "ERROR: Unsupported file format" << std::endl;
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
    std::cerr << "ERROR: Failed to write output file: " << output_file << std::endl;
    return false;
  }

  return true;
}

}  // namespace neeva
