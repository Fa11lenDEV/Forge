#include "forge_cli/args.h"
#include "forge_platform/fs.h"
#include "forge_platform/path.h"

#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static std::filesystem::path remotes_dir(const std::filesystem::path& wd) {
  return wd / ".forge" / "remotes";
}

int remote(const forge_cli::ParsedArgs& a) {
  auto sub = a.positionals.empty() ? "list" : a.positionals[0];
  auto wd = forge_platform::path::cwd();
  std::string err;

  if (sub == "add") {
    if (a.positionals.size() < 3) {
      std::cerr << "forge remote add: use <name> <path>\n";
      return 2;
    }
    auto name = a.positionals[1];
    auto path = a.positionals[2];
    if (!forge_platform::fs::ensure_dir(remotes_dir(wd))) {
      std::cerr << "forge remote: failed to create remotes dir\n";
      return 1;
    }
    if (!forge_platform::fs::write_file_atomic(remotes_dir(wd) / name, path + "\n")) {
      std::cerr << "forge remote: failed to write remote\n";
      return 1;
    }
    return 0;
  }

  if (sub == "list") {
    std::error_code ec;
    auto dir = remotes_dir(wd);
    if (!std::filesystem::exists(dir, ec)) return 0;
    for (auto& it : std::filesystem::directory_iterator(dir, ec)) {
      if (!it.is_regular_file()) continue;
      auto raw = forge_platform::fs::read_file(it.path());
      if (!raw) continue;
      std::cout << it.path().filename().string() << "\t" << *raw;
      if (!raw->empty() && raw->back() != '\n') std::cout << "\n";
    }
    return 0;
  }

  std::cerr << "forge remote: unknown subcommand\n";
  return 2;
}

}

