#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace forge_core::diff {

std::vector<std::string> diff_index_vs_worktree(const std::filesystem::path& workdir, std::string* err);

}

