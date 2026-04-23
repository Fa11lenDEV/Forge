#include "forge_cli/args.h"
#include "forge_core/merge/merge.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int merge(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge merge: provide a branch\n";
    return 2;
  }
  auto branch = a.positionals[0];
  auto msg = a.option("m");
  if (msg.empty()) msg = "merge " + branch;

  std::string err;
  auto wd = forge_platform::path::cwd();
  auto hex = forge_core::merge::merge_branch(wd, branch, msg, &err);
  if (hex.empty()) {
    std::cerr << "forge merge: " << err << "\n";
    return 1;
  }
  std::cout << hex << "\n";
  return 0;
}

}

