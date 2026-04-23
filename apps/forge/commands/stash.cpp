#include "forge_cli/args.h"
#include "forge_core/stash/stash.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int stash(const forge_cli::ParsedArgs& a) {
  auto sub = a.positionals.empty() ? "push" : a.positionals[0];
  std::string err;
  auto wd = forge_platform::path::cwd();

  if (sub == "push") {
    auto msg = a.option("m");
    if (msg.empty()) msg = "wip";
    std::string hex;
    if (!forge_core::stash::push(wd, msg, &hex, &err)) {
      std::cerr << "forge stash: " << err << "\n";
      return 1;
    }
    std::cout << hex << "\n";
    return 0;
  }

  if (sub == "list") {
    auto top = forge_core::stash::read_top(wd, &err);
    if (!top) {
      std::cout << "(empty)\n";
      return 0;
    }
    std::cout << *top << "\n";
    return 0;
  }

  if (sub == "pop") {
    if (!forge_core::stash::pop(wd, &err)) {
      std::cerr << "forge stash: " << err << "\n";
      return 1;
    }
    return 0;
  }

  if (sub == "apply") {
    if (!forge_core::stash::apply(wd, &err)) {
      std::cerr << "forge stash: " << err << "\n";
      return 1;
    }
    return 0;
  }

  std::cerr << "forge stash: unknown subcommand\n";
  return 2;
}

}

