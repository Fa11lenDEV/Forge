#include "forge_core/crypto/credentials.h"

#include "forge_core/crypto/dpapi.h"
#include "forge_platform/fs.h"

#include <filesystem>

namespace forge_core::crypto::credentials {

static std::filesystem::path cred_path(const std::filesystem::path& workdir, const std::string& name) {
  return workdir / ".forge" / "credentials" / (name + ".dpapi");
}

bool set_token(const std::filesystem::path& workdir, const std::string& remote_name, const std::string& token, std::string* err) {
  std::vector<std::uint8_t> pt(token.begin(), token.end());
  auto blob = forge_core::crypto::dpapi::protect(pt, err);
  if (!blob) return false;
  auto p = cred_path(workdir, remote_name);
  if (!forge_platform::fs::ensure_dir(p.parent_path())) {
    if (err) *err = "failed to create credentials dir";
    return false;
  }
  std::string s(reinterpret_cast<const char*>(blob->data()), blob->size());
  if (!forge_platform::fs::write_file_atomic(p, s)) {
    if (err) *err = "failed to write credential";
    return false;
  }
  return true;
}

std::optional<std::string> get_token(const std::filesystem::path& workdir, const std::string& remote_name, std::string* err) {
  auto p = cred_path(workdir, remote_name);
  auto raw = forge_platform::fs::read_file(p);
  if (!raw) return std::nullopt;
  std::vector<std::uint8_t> blob(raw->begin(), raw->end());
  auto pt = forge_core::crypto::dpapi::unprotect(blob, err);
  if (!pt) return std::nullopt;
  return std::string((const char*)pt->data(), pt->size());
}

bool delete_token(const std::filesystem::path& workdir, const std::string& remote_name, std::string* err) {
  std::error_code ec;
  auto p = cred_path(workdir, remote_name);
  std::filesystem::remove(p, ec);
  if (ec) {
    if (err) *err = "failed to delete credential";
    return false;
  }
  return true;
}

}

