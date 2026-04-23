#include "forge_server/handlers.h"

#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

#include <filesystem>

namespace forge_server {

InfoRefsResult info_refs(const std::filesystem::path& repo_dir, std::string* err) {
  InfoRefsResult out;
  if (!forge_core::repo::is_repo(repo_dir)) {
    if (err) *err = "not a forge repo";
    return out;
  }

  auto refs_dir = repo_dir / ".forge" / "refs" / "heads";
  std::error_code ec;
  if (!std::filesystem::exists(refs_dir, ec)) return out;

  for (auto& it : std::filesystem::directory_iterator(refs_dir, ec)) {
    if (!it.is_regular_file()) continue;
    auto raw = forge_platform::fs::read_file(it.path());
    if (!raw) continue;
    out.text.append(it.path().filename().string());
    out.text.push_back('\t');
    out.text.append(*raw);
    if (!out.text.empty() && out.text.back() != '\n') out.text.push_back('\n');
  }
  return out;
}

}

