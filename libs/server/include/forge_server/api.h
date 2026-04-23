#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace forge_server::api {

struct Branch {
  std::string name;
  std::string tip_hex;
};

struct CommitView {
  std::string hex;
  std::string message;
  std::optional<std::string> parent;
};

std::vector<Branch> list_branches(const std::filesystem::path& repo_dir, std::string* err);
std::optional<CommitView> read_commit(const std::filesystem::path& repo_dir, const std::string& hex, std::string* err);
std::optional<std::string> read_blob(const std::filesystem::path& repo_dir, const std::string& hex, std::string* err);
std::optional<std::string> read_tree_raw(const std::filesystem::path& repo_dir, const std::string& hex, std::string* err);

}

