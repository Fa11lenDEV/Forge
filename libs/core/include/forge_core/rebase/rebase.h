#pragma once

#include <filesystem>
#include <string>

namespace forge_core::rebase {

bool rebase_onto(const std::filesystem::path& workdir, const std::string& upstream_branch, bool hard, std::string* err);

}

