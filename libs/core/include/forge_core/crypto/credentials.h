#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace forge_core::crypto::credentials {

bool set_token(const std::filesystem::path& workdir, const std::string& remote_name, const std::string& token, std::string* err);
std::optional<std::string> get_token(const std::filesystem::path& workdir, const std::string& remote_name, std::string* err);
bool delete_token(const std::filesystem::path& workdir, const std::string& remote_name, std::string* err);

}

