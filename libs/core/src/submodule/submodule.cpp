#include "forge_core/submodule/submodule.h"

#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

namespace forge_core::submodule {

static std::filesystem::path submodules_file(const std::filesystem::path& workdir) {
  return forge_core::repo::make_paths(workdir).forge_dir / "submodules";
}

static std::string trim(std::string s) {
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
  return s;
}

std::vector<Entry> list(const std::filesystem::path& workdir, std::string* err) {
  std::vector<Entry> out;
  auto raw = forge_platform::fs::read_file(submodules_file(workdir));
  if (!raw) return out;

  std::string_view s(*raw);
  size_t pos = 0;
  while (pos < s.size()) {
    auto eol = s.find('\n', pos);
    if (eol == std::string_view::npos) eol = s.size();
    auto line = s.substr(pos, eol - pos);
    auto tab = line.find('\t');
    if (tab != std::string_view::npos) {
      Entry e;
      e.path = std::string(line.substr(0, tab));
      e.url = std::string(line.substr(tab + 1));
      if (!e.path.empty() && !e.url.empty()) out.push_back(std::move(e));
    }
    pos = eol + 1;
  }
  return out;
}

bool add(const std::filesystem::path& workdir, const std::string& url, const std::string& path, std::string* err) {
  auto cur = list(workdir, err);
  for (const auto& e : cur) {
    if (e.path == path) {
      if (err) *err = "submodule already exists";
      return false;
    }
  }

  std::string out;
  for (const auto& e : cur) {
    out.append(e.path);
    out.push_back('\t');
    out.append(e.url);
    out.push_back('\n');
  }
  out.append(path);
  out.push_back('\t');
  out.append(url);
  out.push_back('\n');

  if (!forge_platform::fs::write_file_atomic(submodules_file(workdir), out)) {
    if (err) *err = "failed to write submodules";
    return false;
  }
  return true;
}

}

