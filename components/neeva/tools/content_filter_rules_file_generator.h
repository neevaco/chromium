// Copyright 2022 Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_TOOLS_CONTENT_FILTER_RULES_FILE_GENERATOR_H_
#define COMPONENTS_NEEVA_TOOLS_CONTENT_FILTER_RULES_FILE_GENERATOR_H_

namespace base {
class FilePath;
}

namespace neeva {

class ContentFilterRulesFileGenerator {
 public:
  static bool Generate(const base::FilePath& input_file,
                       const base::FilePath& output_file);
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_TOOLS_CONTENT_FILTER_RULES_FILE_GENERATOR_H_
