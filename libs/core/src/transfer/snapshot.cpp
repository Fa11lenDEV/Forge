#include "forge_core/transfer/snapshot.h"

#include "forge_platform/fs.h"

#include <filesystem>

namespace forge_core::transfer {

static bool is_inside_forge(const std::filesystem::path& rel) {
  for (auto it = rel.begin(); it != rel.end(); ++it) {
    if (it->string() == ".forge") return true;
  }
  return false;
}

std::vector<FileEntry> snapshot_forge_dir(const std::filesystem::path& repo_dir, std::string* err) {
  std::vector<FileEntry> out;
  auto forge_dir = repo_dir / ".forge";
  std::error_code ec;
  if (!std::filesystem::exists(forge_dir, ec)) return out;

  for (auto& it : std::filesystem::recursive_directory_iterator(forge_dir, ec)) {
    if (it.is_directory()) continue;
    if (!it.is_regular_file()) continue;
    auto rel = it.path().lexically_relative(repo_dir).generic_string();
    auto raw = forge_platform::fs::read_file(it.path());
    if (!raw) {
      if (err) *err = "failed to read file";
      return {};
    }
    out.push_back({rel, *raw});
  }
  return out;
}

bool apply_snapshot(const std::filesystem::path& repo_dir, const std::vector<FileEntry>& entries, std::string* err) {
  for (const auto& e : entries) {
    auto rel = std::filesystem::path(e.rel_path);
    if (!is_inside_forge(rel)) continue;
    auto out_path = repo_dir / rel;
    if (!forge_platform::fs::ensure_dir(out_path.parent_path())) {
      if (err) *err = "failed to create directory";
      return false;
    }
    if (!forge_platform::fs::write_file_atomic(out_path, e.bytes)) {
      if (err) *err = "failed to write file";
      return false;
    }
  }
  return true;
}

}

