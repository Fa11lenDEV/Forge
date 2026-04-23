#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace forge_core::commit {

struct CommitResult {
  std::string commit_hex;
};

CommitResult create_commit_from_index(const std::filesystem::path& workdir, const std::string& message, std::string* err);
std::optional<std::string> read_commit_parent(const std::filesystem::path& workdir, const std::string& commit_hex, std::string* err);
std::optional<std::string> read_commit_message(const std::filesystem::path& workdir, const std::string& commit_hex, std::string* err);

}

