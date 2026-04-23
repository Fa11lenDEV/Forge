#include "forge_cli/args.h"
#include "forge_core/config/config.h"

#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static void print_map(const forge_core::config::Map& m) {
  for (const auto& [k, v] : m) std::cout << k << "=" << v << "\n";
}

int config(const forge_cli::ParsedArgs& a) {
  auto global = a.has_flag("global");
  auto list = a.has_flag("list");
  auto wd = std::filesystem::current_path();
  std::string err;

  if (list) {
    if (global) {
      auto m = forge_core::config::load_global(&err);
      if (!m) { std::cerr << "forge config: " << err << "\n"; return 1; }
      print_map(*m);
      return 0;
    }
    auto rm = forge_core::config::load_repo(wd, &err);
    if (!rm) { std::cerr << "forge config: " << err << "\n"; return 1; }
    auto gm = forge_core::config::load_global(&err);
    if (!gm) { std::cerr << "forge config: " << err << "\n"; return 1; }
    forge_core::config::Map merged = *gm;
    for (const auto& [k, v] : *rm) merged[k] = v;
    print_map(merged);
    return 0;
  }

  if (a.positionals.size() < 2) {
    std::cerr << "forge config: usage: forge config [--global] <key> <value>\n";
    return 2;
  }
  auto key = a.positionals[0];
  auto value = a.positionals[1];

  if (global) {
    if (!forge_core::config::set_global(key, value, &err)) {
      std::cerr << "forge config: " << err << "\n";
      return 1;
    }
    return 0;
  }

  if (!forge_core::config::set_repo(wd, key, value, &err)) {
    std::cerr << "forge config: " << err << "\n";
    return 1;
  }
  return 0;
}

}

