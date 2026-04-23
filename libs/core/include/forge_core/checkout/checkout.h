#pragma once

#include <filesystem>
#include <string>

namespace forge_core::checkout {

bool checkout_commit(const std::filesystem::path& workdir, const std::string& commit_hex, std::string* err);

}

