#include "forge_cli/args.h"
#include "forge_core/commit/commit.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/path.h"

#include <iostream>
#include <optional>
#include <string>

namespace forge_app::commands {

int log(const forge_cli::ParsedArgs& a) {
  auto oneline = a.has_flag("oneline");
  std::string err;
  auto wd = forge_platform::path::cwd();
  auto cur = forge_core::refs::resolve_head_commit(wd, &err);
  if (!cur || cur->empty()) {
    std::cout << "No commits.\n";
    return 0;
  }

  for (int i = 0; i < 2000 && cur && !cur->empty(); ++i) {
    auto msg = forge_core::commit::read_commit_message(wd, *cur, &err);
    if (oneline) {
      std::cout << cur->substr(0, 12);
      if (msg) std::cout << " " << *msg;
      std::cout << "\n";
    } else {
      std::cout << "commit " << *cur << "\n";
      if (msg) std::cout << "    " << *msg << "\n";
      std::cout << "\n";
    }
    cur = forge_core::commit::read_commit_parent(wd, *cur, &err);
  }
  return 0;
}

}

