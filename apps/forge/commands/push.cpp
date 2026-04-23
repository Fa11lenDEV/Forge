#include "forge_cli/args.h"
#include "forge_core/remote/transport.h"
#include "forge_core/remote/url.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/fs.h"
#include "forge_platform/path.h"

#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static std::optional<std::string> read_remote_path(const std::filesystem::path& wd, const std::string& name) {
  return forge_platform::fs::read_file(wd / ".forge" / "remotes" / name);
}

static bool copy_objects_all(const std::filesystem::path& from_forge, const std::filesystem::path& to_forge, std::string* err) {
  std::error_code ec;
  auto from = from_forge / "objects";
  auto to = to_forge / "objects";
  std::filesystem::create_directories(to, ec);
  if (!std::filesystem::exists(from, ec)) return true;
  for (auto& it : std::filesystem::recursive_directory_iterator(from, ec)) {
    if (it.is_directory()) continue;
    auto rel = it.path().lexically_relative(from);
    auto dst = to / rel;
    std::filesystem::create_directories(dst.parent_path(), ec);
    std::filesystem::copy_file(it.path(), dst, std::filesystem::copy_options::skip_existing, ec);
    ec.clear();
  }
  return true;
}

int push(const forge_cli::ParsedArgs& a) {
  auto name = a.positionals.empty() ? "origin" : a.positionals[0];
  auto wd = forge_platform::path::cwd();
  auto rp = read_remote_path(wd, name);
  if (!rp) {
    std::cerr << "forge push: remote not found\n";
    return 1;
  }
  auto url = std::string(*rp);
  auto parsed = forge_core::remote::parse_url(url);
  if (parsed && (parsed->scheme == forge_core::remote::Scheme::Http || parsed->scheme == forge_core::remote::Scheme::Https)) {
    forge_core::remote::RemoteSpec spec;
    spec.url = url;
    spec.token = a.option("token");
    auto r = forge_core::remote::push_from_repo(wd, spec);
    if (!r.ok) {
      std::cerr << "forge push: " << r.err << "\n";
      return 1;
    }
    return 0;
  }

  auto remote = std::filesystem::path(url);
  if (remote.is_relative()) remote = (wd / remote).lexically_normal();

  std::string err;
  if (!copy_objects_all(wd / ".forge", remote / ".forge", &err)) {
    std::cerr << "forge push: " << err << "\n";
    return 1;
  }

  auto head = forge_core::refs::read_head(wd, &err);
  auto cur = forge_core::refs::resolve_head_commit(wd, &err);
  if (!head || !head->is_ref || !cur || cur->empty()) {
    std::cerr << "forge push: nothing to push\n";
    return 1;
  }

  auto branch = head->value;
  if (!forge_platform::fs::ensure_dir((remote / ".forge" / std::filesystem::path(branch)).parent_path())) {
    std::cerr << "forge push: failed to create remote refs\n";
    return 1;
  }
  if (!forge_platform::fs::write_file_atomic(remote / ".forge" / std::filesystem::path(branch), *cur + "\n")) {
    std::cerr << "forge push: failed to update remote ref\n";
    return 1;
  }
  return 0;
}

}

