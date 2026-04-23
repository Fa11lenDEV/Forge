#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace forge_format::pack {

bool repack_loose_objects(const std::filesystem::path& forge_dir, std::string* err);
std::optional<std::string> read_object_from_packs(const std::filesystem::path& forge_dir, std::string_view hex, std::string* err);

}

