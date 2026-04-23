#include "forge_core/repo/repo.h"

#include "forge_platform/fs.h"

namespace forge_core::repo {

RepoPaths make_paths(const std::filesystem::path& workdir) {
  RepoPaths p;
  p.workdir = workdir;
  p.forge_dir = workdir / ".forge";
  p.objects_dir = p.forge_dir / "objects";
  p.packs_dir = p.forge_dir / "packs";
  p.refs_dir = p.forge_dir / "refs";
  p.heads_dir = p.refs_dir / "heads";
  p.tags_dir = p.refs_dir / "tags";
  p.head_file = p.forge_dir / "HEAD";
  p.config_file = p.forge_dir / "config";
  p.locks_dir = p.forge_dir / "locks";
  p.index_file = p.forge_dir / "index";
  return p;
}

bool is_repo(const std::filesystem::path& workdir) {
  auto p = make_paths(workdir);
  return forge_platform::fs::exists(p.forge_dir) && forge_platform::fs::exists(p.head_file);
}

static std::string default_config() {
  return
R"(core.formatversion=1
core.hash=blake3
core.compress=zstd
)";
}

bool init(const std::filesystem::path& workdir, std::string* err) {
  auto p = make_paths(workdir);

  if (forge_platform::fs::exists(p.forge_dir)) {
    if (err) *err = "repository already exists (.forge/)";
    return false;
  }

  if (!forge_platform::fs::ensure_dir(p.objects_dir)) { if (err) *err = "failed to create .forge/objects"; return false; }
  if (!forge_platform::fs::ensure_dir(p.packs_dir)) { if (err) *err = "failed to create .forge/packs"; return false; }
  if (!forge_platform::fs::ensure_dir(p.heads_dir)) { if (err) *err = "failed to create .forge/refs/heads"; return false; }
  if (!forge_platform::fs::ensure_dir(p.tags_dir)) { if (err) *err = "failed to create .forge/refs/tags"; return false; }
  if (!forge_platform::fs::ensure_dir(p.locks_dir)) { if (err) *err = "failed to create .forge/locks"; return false; }

  if (!forge_platform::fs::write_file_atomic(p.head_file, "ref: refs/heads/main\n")) { if (err) *err = "failed to write .forge/HEAD"; return false; }
  if (!forge_platform::fs::write_file_atomic(p.config_file, default_config())) { if (err) *err = "failed to write .forge/config"; return false; }
  if (!forge_platform::fs::write_file_atomic(p.index_file, "")) { if (err) *err = "failed to create .forge/index"; return false; }
  return true;
}

}

