#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace forge_server {

struct FileEntry {
  std::string rel_path;
  std::string bytes;
};

struct InfoRefsResult {
  std::string text;
};

InfoRefsResult info_refs(const std::filesystem::path& repo_dir, std::string* err);
std::vector<FileEntry> snapshot_forge_dir(const std::filesystem::path& repo_dir, std::string* err);
bool apply_snapshot(const std::filesystem::path& repo_dir, const std::vector<FileEntry>& entries, std::string* err);

std::string encode_frames(const std::vector<FileEntry>& entries);
std::optional<std::vector<FileEntry>> decode_frames(std::string_view bytes, std::string* err);

}

