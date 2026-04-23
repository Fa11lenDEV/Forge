#include "forge_cli/args.h"
#include "forge_core/checkout/checkout.h"
#include "forge_core/refs/refs.h"

#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static bool copy_worktree(const std::filesystem::path& from, const std::filesystem::path& to, std::string* err) {
  std::error_code ec;
  std::filesystem::create_directories(to, ec);
  if (ec) { if (err) *err = "failed to create destination"; return false; }

  for (auto& it : std::filesystem::recursive_directory_iterator(from, ec)) {
    if (ec) break;
    auto rel = it.path().lexically_relative(from);
    if (rel.empty()) continue;
    if (rel.begin()->string() == ".forge") continue;
    auto dst = to / rel;
    if (it.is_directory()) {
      std::filesystem::create_directories(dst, ec);
      ec.clear();
      continue;
    }
    if (it.is_regular_file()) {
      std::filesystem::create_directories(dst.parent_path(), ec);
      std::filesystem::copy_file(it.path(), dst, std::filesystem::copy_options::overwrite_existing, ec);
      ec.clear();
    }
  }
  return true;
}

int export_git(const forge_cli::ParsedArgs& a) {
  auto dest = a.positionals.empty() ? std::filesystem::path("export-git") : std::filesystem::path(a.positionals[0]);
  auto wd = std::filesystem::current_path();
  if (!std::filesystem::exists(wd / ".forge")) {
    std::cerr << "forge export-git: not a forge repo\n";
    return 1;
  }

  std::string err;
  std::error_code ec;
  auto force = a.has_flag("force");
  if (std::filesystem::exists(dest, ec)) {
    auto norm = dest.lexically_normal();
    if (!force) {
      std::cerr << "forge export-git: destination exists (use --force)\n";
      return 2;
    }
    if (norm.empty() || norm == norm.root_path()) {
      std::cerr << "forge export-git: refusing dangerous destination\n";
      return 1;
    }
    std::filesystem::remove_all(dest, ec);
  }
  std::filesystem::create_directories(dest, ec);

  if (!copy_worktree(wd, dest, &err)) {
    std::cerr << "forge export-git: " << err << "\n";
    return 1;
  }

  std::cout << "Exported working tree into " << dest.string() << "\n";
  std::cout << "To turn it into a Git repo, run:\n";
  std::cout << "  git -C \"" << dest.string() << "\" init\n";
  std::cout << "  git -C \"" << dest.string() << "\" add -A\n";
  std::cout << "  git -C \"" << dest.string() << "\" commit -m \"export from forge\"\n";

  return 0;
}

}

