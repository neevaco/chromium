// Copyright 2022 Neeva. All rights reserved.

#ifndef COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_RULES_FILE_GENERATOR_H_
#define COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_RULES_FILE_GENERATOR_H_

#include "base/callback_forward.h"

namespace base {
class FilePath;
}

namespace neeva {

class ContentFilterRulesFileGenerator {
 public:
  static bool GenerateNow(const base::FilePath& input_file,
                          const base::FilePath& output_file);

  static void Generate(const base::FilePath& input_file,
                       const base::FilePath& output_file,
                       base::OnceCallback<void(bool)> callback);
};

}  // namespace neeva

#endif  // COMPONENTS_NEEVA_BROWSER_CONTENT_FILTER_RULES_FILE_GENERATOR_H_
