#pragma once

#include "forge_core/transfer/frames.h"

#include <filesystem>
#include <string>
#include <vector>

namespace forge_core::transfer {

std::vector<FileEntry> snapshot_forge_dir(const std::filesystem::path& repo_dir, std::string* err);
bool apply_snapshot(const std::filesystem::path& repo_dir, const std::vector<FileEntry>& entries, std::string* err);

}

