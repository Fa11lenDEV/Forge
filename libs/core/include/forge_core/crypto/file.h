#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace forge_core::crypto::file {

std::optional<std::string> read_repo_file(const std::filesystem::path& workdir, const std::filesystem::path& abs_path, std::string* err);
bool write_repo_file_atomic(const std::filesystem::path& workdir, const std::filesystem::path& abs_path, const std::string& plaintext, std::string* err);

}

