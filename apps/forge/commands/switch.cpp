#include "forge_cli/args.h"
#include "forge_core/checkout/checkout.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int sw(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge switch: provide a branch\n";
    return 2;
  }
  auto branch = a.positionals[0];

  std::string err;
  auto wd = forge_platform::path::cwd();
  auto ref = "refs/heads/" + branch;
  auto commit = forge_core::refs::read_ref(wd, ref, &err);
  if (!commit || commit->empty()) {
    std::cerr << "forge switch: branch not found\n";
    return 1;
  }

  if (!forge_core::refs::write_head_ref(wd, ref, &err)) {
    std::cerr << "forge switch: " << err << "\n";
    return 1;
  }

  if (!forge_core::checkout::checkout_commit(wd, *commit, &err)) {
    std::cerr << "forge switch: " << err << "\n";
    return 1;
  }

  return 0;
}

}

