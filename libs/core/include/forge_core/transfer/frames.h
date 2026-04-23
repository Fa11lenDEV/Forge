#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace forge_core::transfer {

struct FileEntry {
  std::string rel_path;
  std::string bytes;
};

std::string encode_frames(const std::vector<FileEntry>& entries);
std::optional<std::vector<FileEntry>> decode_frames(std::string_view bytes, std::string* err);

}

