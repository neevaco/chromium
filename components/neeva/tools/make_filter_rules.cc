// Copyright 2022 Neeva. All rights reserved.

#include <stdio.h>
#include "base/files/file_path.h"
#include "weblayer/browser/neeva/content_filter_rules_file_generator.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Usage: make_filter_rules INPUT_FILE OUTPUT_FILE\n");
    return -1;
  }
  printf("Generating %s from %s...\n", argv[2], argv[1]);

  base::FilePath input_file(argv[1]);
  base::FilePath output_file(argv[2]);
  if (!weblayer::neeva::ContentFilterRulesFileGenerator::GenerateNow(
          input_file, output_file)) {
    printf("FAILED\n");
    return -1;
  }
  printf("DONE\n");
  return 0;
}
