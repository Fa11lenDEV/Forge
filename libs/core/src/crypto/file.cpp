#include "forge_core/crypto/file.h"

#include "forge_core/crypto/context.h"
#include "forge_platform/fs.h"

namespace forge_core::crypto::file {

std::optional<std::string> read_repo_file(const std::filesystem::path& workdir, const std::filesystem::path& abs_path, std::string* err) {
  auto raw = forge_platform::fs::read_file(abs_path);
  if (!raw) return std::nullopt;
  auto ctx = forge_core::crypto::load_or_create(workdir, err);
  if (!ctx) return std::nullopt;
  if (!ctx->enabled) return *raw;
  return forge_core::crypto::decrypt_bytes(*ctx, *raw, err);
}

bool write_repo_file_atomic(const std::filesystem::path& workdir, const std::filesystem::path& abs_path, const std::string& plaintext, std::string* err) {
  auto ctx = forge_core::crypto::load_or_create(workdir, err);
  if (!ctx) return false;
  std::string out = plaintext;
  if (ctx->enabled) {
    auto enc = forge_core::crypto::encrypt_bytes(*ctx, plaintext, err);
    if (!enc) return false;
    out = *enc;
  }
  return forge_platform::fs::write_file_atomic(abs_path, out);
}

}

