#include "forge_cli/args.h"
#include "forge_core/checkout/checkout.h"
#include "forge_core/index/index.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int reset(const forge_cli::ParsedArgs& a) {
  if (a.positionals.size() >= 2 && a.positionals[0] == "HEAD") {
    std::string err;
    auto wd = forge_platform::path::cwd();
    std::vector<std::string> paths(a.positionals.begin() + 1, a.positionals.end());
    if (!forge_core::index::unstage_paths(wd, paths, &err)) {
      std::cerr << "forge reset: " << err << "\n";
      return 1;
    }
    return 0;
  }

  auto target = a.option("to");
  if (target.empty() && !a.positionals.empty()) target = a.positionals[0];
  if (target.empty()) {
    std::cerr << "forge reset: provide a commit hex (or --to=...)\n";
    return 2;
  }

  std::string err;
  auto wd = forge_platform::path::cwd();
  auto head = forge_core::refs::read_head(wd, &err);
  if (!head || !head->is_ref) {
    std::cerr << "forge reset: detached HEAD not supported yet\n";
    return 1;
  }

  if (!forge_core::refs::write_ref(wd, head->value, target, &err)) {
    std::cerr << "forge reset: " << err << "\n";
    return 1;
  }

  auto hard = a.has_flag("hard");
  if (hard) {
    if (!forge_core::checkout::checkout_commit(wd, target, &err)) {
      std::cerr << "forge reset: " << err << "\n";
      return 1;
    }
  }
  return 0;
}

}

