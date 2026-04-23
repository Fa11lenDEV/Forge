#include "forge_core/merge/merge.h"

#include "forge_core/commit/commit.h"
#include "forge_core/objectstore/objectstore.h"
#include "forge_core/refs/refs.h"

namespace forge_core::merge {

static std::string build_merge_commit(const std::string& tree_hex, const std::string& p1, const std::string& p2, const std::string& message) {
  std::string out;
  out.append("tree ");
  out.append(tree_hex);
  out.push_back('\n');
  out.append("parent ");
  out.append(p1);
  out.push_back('\n');
  out.append("parent ");
  out.append(p2);
  out.push_back('\n');
  out.append("author unknown 0\n");
  out.append("committer unknown 0\n");
  out.push_back('\n');
  out.append(message);
  out.push_back('\n');
  return out;
}

std::string merge_branch(const std::filesystem::path& workdir, const std::string& branch, const std::string& message, std::string* err) {
  auto cur = forge_core::refs::resolve_head_commit(workdir, err);
  if (!cur || cur->empty()) {
    if (err) *err = "no current commit";
    return {};
  }

  auto other = forge_core::refs::read_ref(workdir, "refs/heads/" + branch, err);
  if (!other || other->empty()) {
    if (err) *err = "branch not found";
    return {};
  }

  std::string cerr;
  auto base_commit = forge_core::commit::create_commit_from_index(workdir, message, &cerr);
  if (base_commit.commit_hex.empty()) {
    if (err) *err = cerr;
    return {};
  }

  auto obj = forge_core::objectstore::load_loose(workdir, base_commit.commit_hex, &cerr);
  if (!obj) {
    if (err) *err = "intermediate commit missing";
    return {};
  }

  auto tree_hex_opt = [&]() -> std::optional<std::string> {
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
    return std::nullopt;
  }();

  if (!tree_hex_opt) {
    if (err) *err = "commit has no tree";
    return {};
  }

  auto merge_data = build_merge_commit(*tree_hex_opt, *cur, *other, message);
  auto merge_id = forge_core::objectstore::store_loose(workdir, forge_core::objectstore::ObjectType::Commit, merge_data, &cerr);
  if (merge_id.hex.empty()) {
    if (err) *err = cerr;
    return {};
  }

  auto head = forge_core::refs::read_head(workdir, err);
  if (!head || !head->is_ref) {
    if (err) *err = "detached HEAD not supported yet";
    return {};
  }
  if (!forge_core::refs::write_ref(workdir, head->value, merge_id.hex, err)) return {};
  return merge_id.hex;
}

}

