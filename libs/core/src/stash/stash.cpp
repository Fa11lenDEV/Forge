#include "forge_core/stash/stash.h"

#include "forge_core/checkout/checkout.h"
#include "forge_core/commit/commit.h"
#include "forge_core/refs/refs.h"
#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

namespace forge_core::stash {

static std::filesystem::path stash_file(const std::filesystem::path& workdir) {
  return forge_core::repo::make_paths(workdir).forge_dir / "stash";
}

static std::string trim(std::string s) {
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
  return s;
}

std::optional<std::string> read_top(const std::filesystem::path& workdir, std::string* err) {
  auto raw = forge_platform::fs::read_file(stash_file(workdir));
  if (!raw) return std::nullopt;
  auto t = trim(*raw);
  if (t.empty()) return std::nullopt;
  return t;
}

bool push(const std::filesystem::path& workdir, const std::string& message, std::string* out_hex, std::string* err) {
  std::string cerr;
  auto res = forge_core::commit::create_commit_from_index(workdir, "stash: " + message, &cerr);
  if (res.commit_hex.empty()) {
    if (err) *err = cerr;
    return false;
  }
  if (!forge_platform::fs::write_file_atomic(stash_file(workdir), res.commit_hex + "\n")) {
    if (err) *err = "failed to write stash";
    return false;
  }
  if (out_hex) *out_hex = res.commit_hex;
  return true;
}

bool pop(const std::filesystem::path& workdir, std::string* err) {
  auto top = read_top(workdir, err);
  if (!top) {
    if (err) *err = "stash is empty";
    return false;
  }
  if (!forge_core::checkout::checkout_commit(workdir, *top, err)) return false;
  if (!forge_platform::fs::write_file_atomic(stash_file(workdir), "")) {
    if (err) *err = "failed to clear stash";
    return false;
  }
  return true;
}

}

