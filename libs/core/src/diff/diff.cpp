#include "forge_core/diff/diff.h"

#include "forge_core/index/index.h"

namespace forge_core::diff {

std::vector<std::string> diff_index_vs_worktree(const std::filesystem::path& workdir, std::string* err) {
  std::vector<std::string> out;
  auto st = forge_core::index::status(workdir, err);
  for (const auto& l : st) out.push_back(l.code + " " + l.path);
  return out;
}

}

