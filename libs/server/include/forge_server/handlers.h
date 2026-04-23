#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace forge_server {

struct InfoRefsResult {
  std::string text;
};

InfoRefsResult info_refs(const std::filesystem::path& repo_dir, std::string* err);

}

