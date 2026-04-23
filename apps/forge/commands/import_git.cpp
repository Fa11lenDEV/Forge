#include "forge_cli/args.h"
#include "forge_core/commit/commit.h"
#include "forge_core/index/index.h"
#include "forge_core/repo/repo.h"

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
    if (rel.begin()->string() == ".git") continue;
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

int import_git(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge import-git: provide a git repo path\n";
    return 2;
  }
  auto src = std::filesystem::path(a.positionals[0]);
  if (!std::filesystem::exists(src / ".git")) {
    std::cerr << "forge import-git: .git not found\n";
    return 1;
  }

  auto dest = a.positionals.size() >= 2 ? std::filesystem::path(a.positionals[1]) : (src.filename().string() + ".forge");
  std::string err;

  if (!copy_worktree(src, dest, &err)) {
    std::cerr << "forge import-git: " << err << "\n";
    return 1;
  }

  if (!forge_core::repo::init(dest, &err)) {
    std::cerr << "forge import-git: " << err << "\n";
    return 1;
  }

  std::vector<std::string> paths;
  paths.push_back(".");
  if (!forge_core::index::add_paths(dest, paths, &err)) {
    std::cerr << "forge import-git: " << err << "\n";
    return 1;
  }

  auto res = forge_core::commit::create_commit_from_index(dest, "import from git", &err);
  if (res.commit_hex.empty()) {
    std::cerr << "forge import-git: " << err << "\n";
    return 1;
  }

  std::cout << "Imported into " << dest.string() << "\n";
  return 0;
}

}

