#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace forge_platform::fs {

bool exists(const std::filesystem::path& p);
bool ensure_dir(const std::filesystem::path& p);
bool write_file_atomic(const std::filesystem::path& path, const std::string& data);
std::optional<std::string> read_file(const std::filesystem::path& path);

}

