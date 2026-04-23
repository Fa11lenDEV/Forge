#include "forge_cli/args.h"
#include "forge_core/rebase/rebase.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int rebase(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge rebase: provide an upstream branch\n";
    return 2;
  }
  auto upstream = a.positionals[0];
  auto hard = a.has_flag("hard");

  std::string err;
  auto wd = forge_platform::path::cwd();
  if (!forge_core::rebase::rebase_onto(wd, upstream, hard, &err)) {
    std::cerr << "forge rebase: " << err << "\n";
    return 1;
  }
  return 0;
}

}

