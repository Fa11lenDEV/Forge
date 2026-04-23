#include "forge_cli/args.h"
#include "forge_core/checkout/checkout.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/fs.h"

#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static bool copy_dir(const std::filesystem::path& from, const std::filesystem::path& to, std::string* err) {
  std::error_code ec;
  std::filesystem::create_directories(to, ec);
  if (ec) { if (err) *err = "failed to create directory"; return false; }

  for (auto& it : std::filesystem::recursive_directory_iterator(from, ec)) {
    if (ec) break;
    auto rel = it.path().lexically_relative(from);
    auto dst = to / rel;
    if (it.is_directory()) {
      std::filesystem::create_directories(dst, ec);
      if (ec) { if (err) *err = "failed to create directory"; return false; }
      continue;
    }
    if (it.is_regular_file()) {
      std::filesystem::create_directories(dst.parent_path(), ec);
      std::filesystem::copy_file(it.path(), dst, std::filesystem::copy_options::overwrite_existing, ec);
      if (ec) { if (err) *err = "failed to copy file"; return false; }
    }
  }
  return true;
}

int clone(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge clone: provide a remote path\n";
    return 2;
  }
  auto remote_path = std::filesystem::path(a.positionals[0]);
  auto dest = a.positionals.size() >= 2 ? std::filesystem::path(a.positionals[1]) : remote_path.filename();
  if (dest.empty()) dest = "repo";

  std::string err;
  if (!copy_dir(remote_path / ".forge", dest / ".forge", &err)) {
    std::cerr << "forge clone: " << err << "\n";
    return 1;
  }

  auto head_commit = forge_core::refs::resolve_head_commit(dest, &err);
  if (head_commit && !head_commit->empty()) {
    if (!forge_core::checkout::checkout_commit(dest, *head_commit, &err)) {
      std::cerr << "forge clone: " << err << "\n";
      return 1;
    }
  }
  return 0;
}

}

