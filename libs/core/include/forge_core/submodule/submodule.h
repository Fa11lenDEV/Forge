#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace forge_core::submodule {

struct Entry {
  std::string path;
  std::string url;
  std::vector<std::string> sparse_paths;
};

std::vector<Entry> list(const std::filesystem::path& workdir, std::string* err);
bool add(const std::filesystem::path& workdir, const std::string& url, const std::string& path, std::string* err);
bool add_sparse(const std::filesystem::path& workdir, const std::string& url, const std::string& path, 
                const std::vector<std::string>& sparse_paths, std::string* err);
bool update(const std::filesystem::path& workdir, const std::string& path, std::string* err);
bool set_sparse_paths(const std::filesystem::path& workdir, const std::string& path, 
                      const std::vector<std::string>& sparse_paths, std::string* err);

}

