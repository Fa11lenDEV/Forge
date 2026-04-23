#include "forge_core/crypto/context.h"

#include "forge_core/config/config.h"
#include "forge_core/crypto/aead.h"
#include "forge_core/crypto/dpapi.h"
#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

#include <cstdint>
#include <random>

namespace forge_core::crypto {

static std::vector<std::uint8_t> random_key32() {
  std::vector<std::uint8_t> k(32);
  std::random_device rd;
  for (auto& b : k) b = static_cast<std::uint8_t>(rd());
  return k;
}

static std::filesystem::path key_path(const std::filesystem::path& workdir) {
  return forge_core::repo::make_paths(workdir).forge_dir / "keys" / "master.dpapi";
}

static bool is_enabled(const std::filesystem::path& workdir) {
  return forge_core::config::get_effective(workdir, "crypto.enabled", "0") == "1";
}

std::optional<CryptoContext> load_or_create(const std::filesystem::path& workdir, std::string* err) {
  CryptoContext ctx;
  ctx.enabled = is_enabled(workdir);
  if (!ctx.enabled) return ctx;

  auto kp = key_path(workdir);
  auto raw = forge_platform::fs::read_file(kp);
  if (!raw) {
    auto key = random_key32();
    auto blob = dpapi::protect(key, err);
    if (!blob) return std::nullopt;
    if (!forge_platform::fs::ensure_dir(kp.parent_path())) {
      if (err) *err = "failed to create keys dir";
      return std::nullopt;
    }
    std::string s(reinterpret_cast<const char*>(blob->data()), blob->size());
    if (!forge_platform::fs::write_file_atomic(kp, s)) {
      if (err) *err = "failed to write master key";
      return std::nullopt;
    }
    ctx.master_key32 = std::move(key);
    return ctx;
  }

  std::vector<std::uint8_t> blob(raw->begin(), raw->end());
  auto key = dpapi::unprotect(blob, err);
  if (!key) return std::nullopt;
  if (key->size() != 32) {
    if (err) *err = "invalid master key";
    return std::nullopt;
  }
  ctx.master_key32 = std::move(*key);
  return ctx;
}

static std::string pack_blob(const aead::AeadCiphertext& c) {
  std::string out;
  out.append("FENC1");
  auto put_u32 = [&](std::uint32_t v) {
    out.push_back((char)(v & 0xFF));
    out.push_back((char)((v >> 8) & 0xFF));
    out.push_back((char)((v >> 16) & 0xFF));
    out.push_back((char)((v >> 24) & 0xFF));
  };
  put_u32((std::uint32_t)c.nonce.size());
  out.append((const char*)c.nonce.data(), c.nonce.size());
  put_u32((std::uint32_t)c.tag.size());
  out.append((const char*)c.tag.data(), c.tag.size());
  put_u32((std::uint32_t)c.ciphertext.size());
  out.append((const char*)c.ciphertext.data(), c.ciphertext.size());
  return out;
}

static std::optional<aead::AeadCiphertext> unpack_blob(const std::string& s, std::string* err) {
  if (s.size() < 5 || s.rfind("FENC1", 0) != 0) {
    if (err) *err = "not encrypted";
    return std::nullopt;
  }
  size_t off = 5;
  auto get_u32 = [&](std::uint32_t* v) -> bool {
    if (off + 4 > s.size()) return false;
    std::uint32_t x = 0;
    x |= (std::uint32_t)(unsigned char)s[off + 0] << 0;
    x |= (std::uint32_t)(unsigned char)s[off + 1] << 8;
    x |= (std::uint32_t)(unsigned char)s[off + 2] << 16;
    x |= (std::uint32_t)(unsigned char)s[off + 3] << 24;
    off += 4;
    *v = x;
    return true;
  };

  aead::AeadCiphertext c;
  std::uint32_t n = 0, t = 0, d = 0;
  if (!get_u32(&n) || off + n > s.size()) return std::nullopt;
  c.nonce.assign((const std::uint8_t*)s.data() + off, (const std::uint8_t*)s.data() + off + n);
  off += n;
  if (!get_u32(&t) || off + t > s.size()) return std::nullopt;
  c.tag.assign((const std::uint8_t*)s.data() + off, (const std::uint8_t*)s.data() + off + t);
  off += t;
  if (!get_u32(&d) || off + d > s.size()) return std::nullopt;
  c.ciphertext.assign((const std::uint8_t*)s.data() + off, (const std::uint8_t*)s.data() + off + d);
  return c;
}

std::optional<std::string> encrypt_bytes(const CryptoContext& ctx, const std::string& plaintext, std::string* err) {
  if (!ctx.enabled) return plaintext;
  std::vector<std::uint8_t> pt(plaintext.begin(), plaintext.end());
  auto ct = aead::aes256gcm_encrypt(ctx.master_key32, pt, err);
  if (!ct) return std::nullopt;
  return pack_blob(*ct);
}

std::optional<std::string> decrypt_bytes(const CryptoContext& ctx, const std::string& blob, std::string* err) {
  if (!ctx.enabled) return blob;
  auto packed = unpack_blob(blob, err);
  if (!packed) return std::nullopt;
  auto pt = aead::aes256gcm_decrypt(ctx.master_key32, *packed, err);
  if (!pt) return std::nullopt;
  return std::string((const char*)pt->data(), pt->size());
}

}

