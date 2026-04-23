#include "forge_core/config/config.h"
#include "forge_core/crypto/context.h"
#include "forge_core/transfer/frames.h"
#include "forge_core/transfer/snapshot.h"
#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

#include <cassert>
#include <filesystem>
#include <string>

static std::filesystem::path tempdir(const char* name) {
  auto p = std::filesystem::temp_directory_path() / name;
  std::error_code ec;
  std::filesystem::remove_all(p, ec);
  std::filesystem::create_directories(p, ec);
  return p;
}

static void test_traversal_block() {
  auto repo = tempdir("forge_sec_traversal");
  std::string err;
  assert(forge_core::repo::init(repo, &err));

  forge_core::transfer::FileEntry e;
  e.rel_path = ".forge/../pwned.txt";
  e.bytes = "x";
  bool ok = forge_core::transfer::apply_snapshot(repo, {e}, &err);
  assert(!ok);
}

static void test_frame_limits() {
  std::string err;
  std::string bytes;
  bytes.resize(4);
  bytes[0] = (char)0xFF;
  bytes[1] = (char)0xFF;
  bytes[2] = (char)0xFF;
  bytes[3] = (char)0xFF;
  auto d = forge_core::transfer::decode_frames(bytes, &err);
  assert(!d);
}

static void test_crypto_roundtrip() {
  auto repo = tempdir("forge_sec_crypto");
  std::string err;
  assert(forge_core::repo::init(repo, &err));
  forge_core::config::set_repo(repo, "crypto.enabled", "1", &err);
  auto ctx = forge_core::crypto::load_or_create(repo, &err);
  assert(ctx && ctx->enabled);
  auto enc = forge_core::crypto::encrypt_bytes(*ctx, "hello", &err);
  assert(enc);
  auto dec = forge_core::crypto::decrypt_bytes(*ctx, *enc, &err);
  assert(dec && *dec == "hello");
}

int main() {
  test_traversal_block();
  test_frame_limits();
  test_crypto_roundtrip();
  return 0;
}

