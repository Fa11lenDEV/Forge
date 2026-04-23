#pragma once

#include <filesystem>
#include <string>

namespace forge_core::repo {

struct RepoPaths {
  std::filesystem::path workdir;
  std::filesystem::path forge_dir;
  std::filesystem::path objects_dir;
  std::filesystem::path packs_dir;
  std::filesystem::path refs_dir;
  std::filesystem::path heads_dir;
  std::filesystem::path tags_dir;
  std::filesystem::path head_file;
  std::filesystem::path config_file;
  std::filesystem::path locks_dir;
  std::filesystem::path index_file;
};

RepoPaths make_paths(const std::filesystem::path& workdir);
bool is_repo(const std::filesystem::path& workdir);
bool init(const std::filesystem::path& workdir, std::string* err);

}

