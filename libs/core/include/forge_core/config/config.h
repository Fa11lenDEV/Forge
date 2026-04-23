#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace forge_core::config {

using Map = std::unordered_map<std::string, std::string>;

std::filesystem::path global_config_path();
std::optional<Map> load_file(const std::filesystem::path& path, std::string* err);
bool save_file(const std::filesystem::path& path, const Map& m, std::string* err);

std::optional<Map> load_repo(const std::filesystem::path& workdir, std::string* err);
bool set_repo(const std::filesystem::path& workdir, const std::string& key, const std::string& value, std::string* err);

std::optional<Map> load_global(std::string* err);
bool set_global(const std::string& key, const std::string& value, std::string* err);

std::string get_effective(const std::filesystem::path& workdir, const std::string& key, const std::string& def);

}

