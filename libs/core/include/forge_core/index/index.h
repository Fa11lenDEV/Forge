#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace forge_core::index {

struct Entry {
  std::string path;
  std::string blob_hex;
  std::uint64_t mtime_ns = 0;
  std::uint64_t size = 0;
};

struct Index {
  std::unordered_map<std::string, Entry> entries;
};

std::optional<Index> load(const std::filesystem::path& workdir, std::string* err);
bool save(const std::filesystem::path& workdir, const Index& idx, std::string* err);

bool add_paths(const std::filesystem::path& workdir, const std::vector<std::string>& paths, std::string* err);

struct StatusLine {
  std::string code;
  std::string path;
};

std::vector<StatusLine> status(const std::filesystem::path& workdir, std::string* err);

}

