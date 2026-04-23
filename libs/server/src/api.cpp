#include "forge_server/api.h"

#include "forge_core/commit/commit.h"
#include "forge_core/objectstore/objectstore.h"
#include "forge_platform/fs.h"

#include <filesystem>

namespace forge_server::api {

std::vector<Branch> list_branches(const std::filesystem::path& repo_dir, std::string* err) {
  std::vector<Branch> out;
  std::error_code ec;
  auto dir = repo_dir / ".forge" / "refs" / "heads";
  if (!std::filesystem::exists(dir, ec)) return out;
  for (auto& it : std::filesystem::directory_iterator(dir, ec)) {
    if (!it.is_regular_file()) continue;
    auto raw = forge_platform::fs::read_file(it.path());
    if (!raw) continue;
    Branch b;
    b.name = it.path().filename().string();
    b.tip_hex = *raw;
    while (!b.tip_hex.empty() && (b.tip_hex.back() == '\n' || b.tip_hex.back() == '\r')) b.tip_hex.pop_back();
    out.push_back(std::move(b));
  }
  return out;
}

std::optional<CommitView> read_commit(const std::filesystem::path& repo_dir, const std::string& hex, std::string* err) {
  CommitView v;
  v.hex = hex;
  auto msg = forge_core::commit::read_commit_message(repo_dir, hex, err);
  if (!msg) return std::nullopt;
  v.message = *msg;
  v.parent = forge_core::commit::read_commit_parent(repo_dir, hex, err);
  return v;
}

std::optional<std::string> read_blob(const std::filesystem::path& repo_dir, const std::string& hex, std::string* err) {
  auto obj = forge_core::objectstore::load_loose(repo_dir, hex, err);
  if (!obj) return std::nullopt;
  if (obj->type != forge_core::objectstore::ObjectType::Blob) {
    if (err) *err = "not a blob";
    return std::nullopt;
  }
  return obj->data;
}

std::optional<std::string> read_tree_raw(const std::filesystem::path& repo_dir, const std::string& hex, std::string* err) {
  auto obj = forge_core::objectstore::load_loose(repo_dir, hex, err);
  if (!obj) return std::nullopt;
  if (obj->type != forge_core::objectstore::ObjectType::Tree) {
    if (err) *err = "not a tree";
    return std::nullopt;
  }
  return obj->data;
}

}

