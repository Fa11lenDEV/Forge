#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace forge_core::stash {

std::optional<std::string> read_top(const std::filesystem::path& workdir, std::string* err);
bool push(const std::filesystem::path& workdir, const std::string& message, std::string* out_hex, std::string* err);
bool pop(const std::filesystem::path& workdir, std::string* err);
bool apply(const std::filesystem::path& workdir, std::string* err);

}

