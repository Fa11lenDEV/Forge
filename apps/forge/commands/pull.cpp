#include "forge_cli/args.h"
#include "forge_core/checkout/checkout.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/fs.h"
#include "forge_platform/path.h"

#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static std::optional<std::string> read_remote_path(const std::filesystem::path& wd, const std::string& name) {
  return forge_platform::fs::read_file(wd / ".forge" / "remotes" / name);
}

static bool do_fetch(const std::filesystem::path& remote, const std::filesystem::path& wd) {
  std::error_code ec;
  auto from = remote / ".forge" / "objects";
  auto to = wd / ".forge" / "objects";
  if (!std::filesystem::exists(from, ec)) return true;
  for (auto& it : std::filesystem::recursive_directory_iterator(from, ec)) {
    if (!it.is_regular_file()) continue;
    auto rel = it.path().lexically_relative(from);
    auto dst = to / rel;
    std::filesystem::create_directories(dst.parent_path(), ec);
    std::filesystem::copy_file(it.path(), dst, std::filesystem::copy_options::skip_existing, ec);
    ec.clear();
  }
  return true;
}

int pull(const forge_cli::ParsedArgs& a) {
  auto name = a.positionals.empty() ? "origin" : a.positionals[0];
  auto wd = forge_platform::path::cwd();
  auto rp = read_remote_path(wd, name);
  if (!rp) {
    std::cerr << "forge pull: remote not found\n";
    return 1;
  }
  auto remote = std::filesystem::path(*rp);
  if (remote.is_relative()) remote = (wd / remote).lexically_normal();

  if (!do_fetch(remote, wd)) {
    std::cerr << "forge pull: fetch failed\n";
    return 1;
  }

  std::string err;
  auto head = forge_core::refs::read_head(wd, &err);
  if (!head || !head->is_ref) {
    std::cerr << "forge pull: detached HEAD not supported yet\n";
    return 1;
  }
  auto branch_name = std::filesystem::path(head->value).filename().string();
  auto remote_ref = forge_core::refs::read_ref(wd, "refs/remotes/origin/" + branch_name, &err);
  if (!remote_ref || remote_ref->empty()) {
    std::cerr << "forge pull: remote ref not found\n";
    return 1;
  }
  if (!forge_core::refs::write_ref(wd, head->value, *remote_ref, &err)) {
    std::cerr << "forge pull: " << err << "\n";
    return 1;
  }
  if (!forge_core::checkout::checkout_commit(wd, *remote_ref, &err)) {
    std::cerr << "forge pull: " << err << "\n";
    return 1;
  }
  return 0;
}

}

