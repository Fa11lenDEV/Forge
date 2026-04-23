#include "forge_core/refs/refs.h"

#include "forge_core/crypto/file.h"
#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

#include <string_view>

namespace forge_core::refs {

static std::string trim(std::string s) {
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
  size_t i = 0;
  while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
  if (i) s.erase(0, i);
  return s;
}

std::optional<Head> read_head(const std::filesystem::path& workdir, std::string* err) {
  auto p = repo::make_paths(workdir);
  auto raw = forge_core::crypto::file::read_repo_file(workdir, p.head_file, err);
  if (!raw) {
    if (err) *err = "HEAD missing";
    return std::nullopt;
  }
  auto s = trim(*raw);
  Head h;
  constexpr std::string_view k = "ref:";
  if (s.rfind(k, 0) == 0) {
    h.is_ref = true;
    h.value = trim(s.substr(k.size()));
    return h;
  }
  h.is_ref = false;
  h.value = s;
  return h;
}

bool write_head_ref(const std::filesystem::path& workdir, const std::string& ref, std::string* err) {
  auto p = repo::make_paths(workdir);
  auto data = "ref: " + ref + "\n";
  if (!forge_core::crypto::file::write_repo_file_atomic(workdir, p.head_file, data, err)) {
    if (err) *err = "failed to write HEAD";
    return false;
  }
  return true;
}

static std::filesystem::path ref_path(const repo::RepoPaths& p, const std::string& ref) {
  return p.forge_dir / ref;
}

std::optional<std::string> read_ref(const std::filesystem::path& workdir, const std::string& ref, std::string* err) {
  auto p = repo::make_paths(workdir);
  auto rp = ref_path(p, ref);
  auto raw = forge_core::crypto::file::read_repo_file(workdir, rp, err);
  if (!raw) return std::nullopt;
  return trim(*raw);
}

bool write_ref(const std::filesystem::path& workdir, const std::string& ref, const std::string& hex, std::string* err) {
  auto p = repo::make_paths(workdir);
  auto rp = ref_path(p, ref);
  if (!forge_platform::fs::ensure_dir(rp.parent_path())) {
    if (err) *err = "failed to create refs directory";
    return false;
  }
  if (!forge_core::crypto::file::write_repo_file_atomic(workdir, rp, hex + "\n", err)) {
    if (err) *err = "failed to write ref";
    return false;
  }
  return true;
}

std::optional<std::string> resolve_head_commit(const std::filesystem::path& workdir, std::string* err) {
  auto h = read_head(workdir, err);
  if (!h) return std::nullopt;
  if (!h->is_ref) return h->value;
  auto c = read_ref(workdir, h->value, err);
  return c;
}

}

