// Copyright 2022 Neeva. All rights reserved.

#include <iostream>

#include "base/files/file_path.h"
#include "components/neeva/tools/content_filter_rules_file_generator.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: make_filter_rules INPUT_FILE OUTPUT_FILE" << std::endl;
    return -1;
  }
  std::cout << "Generating " << argv[2] << " from " << argv[1] << "..." <<
      std::endl;

  base::FilePath input_file(argv[1]);
  base::FilePath output_file(argv[2]);
  if (!neeva::ContentFilterRulesFileGenerator::Generate(
          input_file, output_file)) {
    std::cerr << "FAILED" << std::endl;
    return -1;
  }
  std::cout << "DONE" << std::endl;
  return 0;
}
