// Copyright 2022 Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_FILE_GENERATOR_H_
#define WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_FILE_GENERATOR_H_

#include "base/callback_forward.h"

namespace base {
class FilePath;
}

namespace weblayer {
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
}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_RULES_FILE_GENERATOR_H_
