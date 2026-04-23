#include "forge_cli/args.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/fs.h"
#include "forge_platform/path.h"

#include <filesystem>
#include <iostream>
#include <vector>

namespace forge_app::commands {

static std::vector<std::string> list_heads(const std::filesystem::path& workdir) {
  std::vector<std::string> out;
  auto dir = workdir / ".forge" / "refs" / "heads";
  std::error_code ec;
  if (!std::filesystem::exists(dir, ec)) return out;
  for (auto& it : std::filesystem::directory_iterator(dir, ec)) {
    if (it.is_regular_file()) out.push_back(it.path().filename().string());
  }
  return out;
}

int branch(const forge_cli::ParsedArgs& a) {
  std::string err;
  auto wd = forge_platform::path::cwd();

  if (a.positionals.empty()) {
    auto head = forge_core::refs::read_head(wd, &err);
    auto branches = list_heads(wd);
    for (const auto& b : branches) {
      auto name = "refs/heads/" + b;
      auto star = (head && head->is_ref && head->value == name) ? "* " : "  ";
      std::cout << star << b << "\n";
    }
    return 0;
  }

  auto new_branch = a.positionals[0];
  auto cur = forge_core::refs::resolve_head_commit(wd, &err);
  if (!cur || cur->empty()) {
    std::cerr << "forge branch: no commit to point to\n";
    return 1;
  }

  if (!forge_core::refs::write_ref(wd, "refs/heads/" + new_branch, *cur, &err)) {
    std::cerr << "forge branch: " << err << "\n";
    return 1;
  }
  return 0;
}

}

