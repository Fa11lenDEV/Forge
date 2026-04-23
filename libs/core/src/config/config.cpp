#include "forge_core/config/config.h"

#include "forge_core/repo/repo.h"
#include "forge_core/crypto/file.h"
#include "forge_platform/fs.h"

#include <cstdlib>
#include <filesystem>
#include <string_view>

namespace forge_core::config {

static std::string trim(std::string s) {
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
  size_t i = 0;
  while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
  if (i) s.erase(0, i);
  return s;
}

std::filesystem::path global_config_path() {
  const char* p = std::getenv("USERPROFILE");
  if (!p) p = std::getenv("HOME");
  std::filesystem::path home = p ? std::filesystem::path(p) : std::filesystem::current_path();
  return home / ".forgeconfig";
}

std::optional<Map> load_file(const std::filesystem::path& path, std::string* err) {
  Map m;
  auto raw = forge_core::crypto::file::read_repo_file(std::filesystem::current_path(), path, err);
  if (!raw) return m;

  std::string_view s(*raw);
  size_t pos = 0;
  while (pos < s.size()) {
    auto eol = s.find('\n', pos);
    if (eol == std::string_view::npos) eol = s.size();
    auto line = std::string(s.substr(pos, eol - pos));
    pos = eol + 1;
    line = trim(line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;
    auto eq = line.find('=');
    if (eq == std::string::npos) continue;
    auto k = trim(line.substr(0, eq));
    auto v = trim(line.substr(eq + 1));
    if (!k.empty()) m[k] = v;
  }
  return m;
}

bool save_file(const std::filesystem::path& path, const Map& m, std::string* err) {
  std::string out;
  for (const auto& [k, v] : m) {
    out.append(k);
    out.push_back('=');
    out.append(v);
    out.push_back('\n');
  }
  if (!forge_platform::fs::ensure_dir(path.parent_path())) {
    if (err) *err = "failed to create config directory";
    return false;
  }
  if (!forge_core::crypto::file::write_repo_file_atomic(std::filesystem::current_path(), path, out, err)) {
    if (err) *err = "failed to write config";
    return false;
  }
  return true;
}

std::optional<Map> load_repo(const std::filesystem::path& workdir, std::string* err) {
  auto p = forge_core::repo::make_paths(workdir);
  return load_file(p.config_file, err);
}

bool set_repo(const std::filesystem::path& workdir, const std::string& key, const std::string& value, std::string* err) {
  auto p = forge_core::repo::make_paths(workdir);
  auto m = load_file(p.config_file, err);
  if (!m) return false;
  (*m)[key] = value;
  return save_file(p.config_file, *m, err);
}

std::optional<Map> load_global(std::string* err) {
  return load_file(global_config_path(), err);
}

bool set_global(const std::string& key, const std::string& value, std::string* err) {
  auto path = global_config_path();
  auto m = load_file(path, err);
  if (!m) return false;
  (*m)[key] = value;
  return save_file(path, *m, err);
}

std::string get_effective(const std::filesystem::path& workdir, const std::string& key, const std::string& def) {
  if (auto rm = load_repo(workdir, nullptr)) {
    auto it = rm->find(key);
    if (it != rm->end()) return it->second;
  }
  if (auto gm = load_global(nullptr)) {
    auto it = gm->find(key);
    if (it != gm->end()) return it->second;
  }
  return def;
}

}

