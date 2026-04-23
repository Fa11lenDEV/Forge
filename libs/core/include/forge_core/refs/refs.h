#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace forge_core::refs {

struct Head {
  bool is_ref = true;
  std::string value;
};

std::optional<Head> read_head(const std::filesystem::path& workdir, std::string* err);
bool write_head_ref(const std::filesystem::path& workdir, const std::string& ref, std::string* err);

std::optional<std::string> read_ref(const std::filesystem::path& workdir, const std::string& ref, std::string* err);
bool write_ref(const std::filesystem::path& workdir, const std::string& ref, const std::string& hex, std::string* err);

std::optional<std::string> resolve_head_commit(const std::filesystem::path& workdir, std::string* err);

}

