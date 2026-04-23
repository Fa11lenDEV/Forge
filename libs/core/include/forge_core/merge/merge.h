#pragma once

#include <filesystem>
#include <string>

namespace forge_core::merge {

std::string merge_branch(const std::filesystem::path& workdir, const std::string& branch, const std::string& message, std::string* err);

}

