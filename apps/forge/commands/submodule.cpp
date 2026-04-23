#include "forge_cli/args.h"
#include "forge_core/submodule/submodule.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int submodule(const forge_cli::ParsedArgs& a) {
  auto sub = a.positionals.empty() ? "status" : a.positionals[0];
  std::string err;
  auto wd = forge_platform::path::cwd();

  if (sub == "add") {
    if (a.positionals.size() < 3) {
      std::cerr << "forge submodule add: use <url> <path>\n";
      return 2;
    }
    auto url = a.positionals[1];
    auto path = a.positionals[2];
    if (!forge_core::submodule::add(wd, url, path, &err)) {
      std::cerr << "forge submodule: " << err << "\n";
      return 1;
    }
    return 0;
  }

  if (sub == "status") {
    auto list = forge_core::submodule::list(wd, &err);
    for (const auto& e : list) {
      std::cout << e.path << " " << e.url << "\n";
    }
    return 0;
  }

  std::cerr << "forge submodule: unknown subcommand\n";
  return 2;
}

}

