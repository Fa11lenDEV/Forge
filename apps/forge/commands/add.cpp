#include "forge_cli/args.h"
#include "forge_core/index/index.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int add(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge add: provide paths\n";
    return 2;
  }

  std::string err;
  auto wd = forge_platform::path::cwd();
  if (!forge_core::index::add_paths(wd, a.positionals, &err)) {
    std::cerr << "forge add: " << err << "\n";
    return 1;
  }
  return 0;
}

}

