#include "forge_core/diff/diff.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int diff() {
  std::string err;
  auto wd = forge_platform::path::cwd();
  auto lines = forge_core::diff::diff_index_vs_worktree(wd, &err);
  if (!err.empty() && lines.empty()) {
    std::cerr << "forge diff: " << err << "\n";
    return 1;
  }
  for (const auto& l : lines) std::cout << l << "\n";
  return 0;
}

}

