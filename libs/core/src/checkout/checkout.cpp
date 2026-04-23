#include "forge_core/checkout/checkout.h"

#include "forge_core/objectstore/objectstore.h"
#include "forge_platform/fs.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace forge_core::checkout {

static std::optional<std::string> read_tree_hex(const std::filesystem::path& workdir, const std::string& commit_hex, std::string* err) {
  auto obj = forge_core::objectstore::load_loose(workdir, commit_hex, err);
  if (!obj) return std::nullopt;
  if (obj->type != forge_core::objectstore::ObjectType::Commit) {
    if (err) *err = "not a commit";
    return std::nullopt;
  }
  std::string_view s(obj->data);
  size_t pos = 0;
  while (pos < s.size()) {
    auto eol = s.find('\n', pos);
    if (eol == std::string_view::npos) break;
    if (eol == pos) break;
    auto line = s.substr(pos, eol - pos);
    constexpr std::string_view k = "tree ";
    if (line.rfind(k, 0) == 0) return std::string(line.substr(k.size()));
    pos = eol + 1;
  }
  if (err) *err = "commit has no tree";
  return std::nullopt;
}

struct TreeEntry {
  std::string path;
  std::string blob_hex;
};

static std::optional<std::vector<TreeEntry>> parse_tree(std::string_view data, std::string* err) {
  std::vector<TreeEntry> out;
  size_t pos = 0;
  while (pos < data.size()) {
    auto sp = data.find(' ', pos);
    if (sp == std::string_view::npos) break;
    auto nul = data.find('\0', sp + 1);
    if (nul == std::string_view::npos) break;
    auto nl = data.find('\n', nul + 1);
    if (nl == std::string_view::npos) break;

    auto path = data.substr(sp + 1, nul - (sp + 1));
    auto hex = data.substr(nul + 1, nl - (nul + 1));
    TreeEntry e;
    e.path = std::string(path);
    e.blob_hex = std::string(hex);
    out.push_back(std::move(e));
    pos = nl + 1;
  }
  if (out.empty() && !data.empty()) {
    if (err) *err = "invalid tree";
    return std::nullopt;
  }
  return out;
}

bool checkout_commit(const std::filesystem::path& workdir, const std::string& commit_hex, std::string* err) {
  auto tree_hex = read_tree_hex(workdir, commit_hex, err);
  if (!tree_hex) return false;

  auto tree_obj = forge_core::objectstore::load_loose(workdir, *tree_hex, err);
  if (!tree_obj || tree_obj->type != forge_core::objectstore::ObjectType::Tree) {
    if (err && err->empty()) *err = "tree not found";
    return false;
  }

  auto entries = parse_tree(tree_obj->data, err);
  if (!entries) return false;

  for (const auto& e : *entries) {
    auto blob_obj = forge_core::objectstore::load_loose(workdir, e.blob_hex, err);
    if (!blob_obj || blob_obj->type != forge_core::objectstore::ObjectType::Blob) return false;

    auto out_path = workdir / std::filesystem::path(e.path);
    if (!forge_platform::fs::ensure_dir(out_path.parent_path())) {
      if (err) *err = "failed to create directory";
      return false;
    }
    if (!forge_platform::fs::write_file_atomic(out_path, blob_obj->data)) {
      if (err) *err = "failed to write file";
      return false;
    }
  }

  return true;
}

}

