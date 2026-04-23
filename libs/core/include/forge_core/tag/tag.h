#pragma once

#include <filesystem>
#include <string>

namespace forge_core::tag {

bool create_lightweight(const std::filesystem::path& workdir, const std::string& name, const std::string& target_hex, std::string* err);

}

