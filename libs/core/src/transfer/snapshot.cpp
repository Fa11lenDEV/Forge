#include "forge_core/transfer/snapshot.h"

#include "forge_platform/fs.h"

#include <filesystem>

namespace forge_core::transfer {

static bool has_parent_refs(const std::filesystem::path& p) {
  for (const auto& part : p) {
    if (part == "..") return true;
  }
  return false;
}

static bool is_safe_relative_forge_path(const std::filesystem::path& rel) {
  if (rel.empty()) return false;
  if (rel.is_absolute()) return false;
  if (has_parent_refs(rel)) return false;
  auto it = rel.begin();
  if (it == rel.end()) return false;
  if (it->string() != ".forge") return false;
  return true;
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
  std::error_code ec;
  auto forge_root = std::filesystem::weakly_canonical(repo_dir / ".forge", ec);
  if (ec) forge_root = (repo_dir / ".forge").lexically_normal();

  for (const auto& e : entries) {
    auto rel = std::filesystem::path(e.rel_path);
    if (!is_safe_relative_forge_path(rel)) continue;

    auto out_path = (repo_dir / rel).lexically_normal();
    auto out_can = std::filesystem::weakly_canonical(out_path, ec);
    if (ec) out_can = out_path;

    auto out_str = out_can.generic_string();
    auto root_str = forge_root.generic_string();
    if (out_str.size() < root_str.size() || out_str.compare(0, root_str.size(), root_str) != 0) {
      if (err) *err = "path traversal blocked";
      return false;
    }

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

