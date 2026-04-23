#include "forge_cli/args.h"
#include "forge_core/checkout/checkout.h"
#include "forge_core/index/index.h"
#include "forge_core/objectstore/objectstore.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/fs.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int sw(const forge_cli::ParsedArgs& a) {
  if (!a.positionals.empty() && a.positionals[0] == "--") {
    if (a.positionals.size() < 2) {
      std::cerr << "forge checkout: provide a path after --\n";
      return 2;
    }
    auto wd = forge_platform::path::cwd();
    std::string err;
    auto rel = std::filesystem::path(a.positionals[1]).generic_string();
    if (!rel.empty() && rel[0] == '/') rel.erase(0, 1);
    auto e = forge_core::index::get_entry(wd, rel, &err);
    if (!e) {
      std::cerr << "forge checkout: not in index\n";
      return 1;
    }
    auto obj = forge_core::objectstore::load_loose(wd, e->blob_hex, &err);
    if (!obj) {
      std::cerr << "forge checkout: " << err << "\n";
      return 1;
    }
    auto out_path = wd / std::filesystem::path(rel);
    if (!forge_platform::fs::ensure_dir(out_path.parent_path())) {
      std::cerr << "forge checkout: failed to create directory\n";
      return 1;
    }
    if (!forge_platform::fs::write_file_atomic(out_path, obj->data)) {
      std::cerr << "forge checkout: failed to write file\n";
      return 1;
    }
    return 0;
  }

  if (a.positionals.empty()) {
    std::cerr << "forge switch: provide a branch\n";
    return 2;
  }
  auto branch = a.positionals[0];
  if (a.has_flag("b")) {
    auto cur = forge_core::refs::resolve_head_commit(forge_platform::path::cwd(), nullptr);
    if (!cur || cur->empty()) {
      std::cerr << "forge switch: no commit to point to\n";
      return 1;
    }
    std::string berr;
    if (!forge_core::refs::write_ref(forge_platform::path::cwd(), "refs/heads/" + branch, *cur, &berr)) {
      std::cerr << "forge switch: " << berr << "\n";
      return 1;
    }
  }

  std::string err;
  auto wd = forge_platform::path::cwd();
  auto ref = "refs/heads/" + branch;
  auto commit = forge_core::refs::read_ref(wd, ref, &err);
  if (!commit || commit->empty()) {
    std::cerr << "forge switch: branch not found\n";
    return 1;
  }

  if (!forge_core::refs::write_head_ref(wd, ref, &err)) {
    std::cerr << "forge switch: " << err << "\n";
    return 1;
  }

  if (!forge_core::checkout::checkout_commit(wd, *commit, &err)) {
    std::cerr << "forge switch: " << err << "\n";
    return 1;
  }

  return 0;
}

}

