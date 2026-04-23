#include "forge_core/commit/commit.h"

#include "forge_core/index/index.h"
#include "forge_core/objectstore/objectstore.h"
#include "forge_core/refs/refs.h"
#include "forge_core/repo/repo.h"

#include <algorithm>
#include <chrono>
#include <string_view>

namespace forge_core::commit {

static std::string now_seconds() {
  using namespace std::chrono;
  auto s = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
  return std::to_string(s);
}

static std::string build_tree(const index::Index& idx) {
  std::vector<index::Entry> v;
  v.reserve(idx.entries.size());
  for (const auto& [k, e] : idx.entries) v.push_back(e);
  std::sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.path < b.path; });

  std::string out;
  for (const auto& e : v) {
    out.append("100644 ");
    out.append(e.path);
    out.push_back('\0');
    out.append(e.blob_hex);
    out.push_back('\n');
  }
  return out;
}

static std::string build_commit(const std::string& tree_hex, const std::optional<std::string>& parent, const std::string& message) {
  std::string out;
  out.append("tree ");
  out.append(tree_hex);
  out.push_back('\n');
  if (parent && !parent->empty()) {
    out.append("parent ");
    out.append(*parent);
    out.push_back('\n');
  }
  out.append("author ");
  out.append("unknown ");
  out.append(now_seconds());
  out.push_back('\n');
  out.append("committer ");
  out.append("unknown ");
  out.append(now_seconds());
  out.push_back('\n');
  out.push_back('\n');
  out.append(message);
  out.push_back('\n');
  return out;
}

CommitResult create_commit_from_index(const std::filesystem::path& workdir, const std::string& message, std::string* err) {
  auto idx_opt = index::load(workdir, err);
  if (!idx_opt) return {};
  auto& idx = *idx_opt;

  auto tree_data = build_tree(idx);
  std::string oerr;
  auto tree_id = objectstore::store_loose(workdir, objectstore::ObjectType::Tree, tree_data, &oerr);
  if (tree_id.hex.empty()) {
    if (err) *err = oerr;
    return {};
  }

  auto parent = refs::resolve_head_commit(workdir, nullptr);
  auto commit_data = build_commit(tree_id.hex, parent, message);
  auto commit_id = objectstore::store_loose(workdir, objectstore::ObjectType::Commit, commit_data, &oerr);
  if (commit_id.hex.empty()) {
    if (err) *err = oerr;
    return {};
  }

  auto head = refs::read_head(workdir, err);
  if (!head) return {};
  if (head->is_ref) {
    if (!refs::write_ref(workdir, head->value, commit_id.hex, err)) return {};
  } else {
    if (err) *err = "detached HEAD not supported yet";
    return {};
  }

  return CommitResult{commit_id.hex};
}

static std::optional<std::string> find_header_value(std::string_view data, std::string_view key) {
  size_t pos = 0;
  while (pos < data.size()) {
    auto eol = data.find('\n', pos);
    if (eol == std::string_view::npos) return std::nullopt;
    if (eol == pos) return std::nullopt;
    auto line = data.substr(pos, eol - pos);
    if (line.rfind(key, 0) == 0) return std::string(line.substr(key.size()));
    pos = eol + 1;
  }
  return std::nullopt;
}

std::optional<std::string> read_commit_parent(const std::filesystem::path& workdir, const std::string& commit_hex, std::string* err) {
  auto obj = objectstore::load_loose(workdir, commit_hex, err);
  if (!obj) return std::nullopt;
  if (obj->type != objectstore::ObjectType::Commit) {
    if (err) *err = "not a commit";
    return std::nullopt;
  }
  auto v = find_header_value(obj->data, "parent ");
  return v;
}

std::optional<std::string> read_commit_message(const std::filesystem::path& workdir, const std::string& commit_hex, std::string* err) {
  auto obj = objectstore::load_loose(workdir, commit_hex, err);
  if (!obj) return std::nullopt;
  if (obj->type != objectstore::ObjectType::Commit) return std::nullopt;
  auto sep = obj->data.find("\n\n");
  if (sep == std::string::npos) return std::nullopt;
  auto msg = obj->data.substr(sep + 2);
  while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) msg.pop_back();
  return msg;
}

}

