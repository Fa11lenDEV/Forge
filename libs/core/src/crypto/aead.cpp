#include "forge_core/crypto/aead.h"

#ifdef _WIN32
#  include <windows.h>
#  include <bcrypt.h>
#  pragma comment(lib, "bcrypt.lib")
#endif

#include <random>

namespace forge_core::crypto::aead {

static std::vector<std::uint8_t> random_bytes(size_t n) {
  std::vector<std::uint8_t> v(n);
  std::random_device rd;
  for (size_t i = 0; i < n; ++i) v[i] = static_cast<std::uint8_t>(rd());
  return v;
}

std::optional<AeadCiphertext> aes256gcm_encrypt(
  const std::vector<std::uint8_t>& key32,
  const std::vector<std::uint8_t>& plaintext,
  std::string* err) {
#ifdef _WIN32
  if (key32.size() != 32) { if (err) *err = "bad key size"; return std::nullopt; }

  BCRYPT_ALG_HANDLE hAlg = nullptr;
  BCRYPT_KEY_HANDLE hKey = nullptr;
  NTSTATUS st = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
  if (st) { if (err) *err = "BCryptOpenAlgorithmProvider failed"; return std::nullopt; }

  st = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
  if (st) { if (err) *err = "BCryptSetProperty failed"; BCryptCloseAlgorithmProvider(hAlg, 0); return std::nullopt; }

  DWORD cbKeyObj = 0, cbData = 0;
  st = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbKeyObj, sizeof(cbKeyObj), &cbData, 0);
  if (st) { if (err) *err = "BCryptGetProperty failed"; BCryptCloseAlgorithmProvider(hAlg, 0); return std::nullopt; }

  std::vector<std::uint8_t> keyObj(cbKeyObj);
  st = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObj.data(), cbKeyObj, (PUCHAR)key32.data(), (ULONG)key32.size(), 0);
  if (st) { if (err) *err = "BCryptGenerateSymmetricKey failed"; BCryptCloseAlgorithmProvider(hAlg, 0); return std::nullopt; }

  AeadCiphertext out;
  out.nonce = random_bytes(12);
  out.tag.resize(16);
  out.ciphertext.resize(plaintext.size());

  BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;
  BCRYPT_INIT_AUTH_MODE_INFO(info);
  info.pbNonce = out.nonce.data();
  info.cbNonce = (ULONG)out.nonce.size();
  info.pbTag = out.tag.data();
  info.cbTag = (ULONG)out.tag.size();

  ULONG cbOut = 0;
  st = BCryptEncrypt(
    hKey,
    (PUCHAR)plaintext.data(),
    (ULONG)plaintext.size(),
    &info,
    nullptr,
    0,
    out.ciphertext.data(),
    (ULONG)out.ciphertext.size(),
    &cbOut,
    0);

  BCryptDestroyKey(hKey);
  BCryptCloseAlgorithmProvider(hAlg, 0);

  if (st) { if (err) *err = "BCryptEncrypt failed"; return std::nullopt; }
  out.ciphertext.resize(cbOut);
  return out;
#else
  if (err) *err = "aead not supported";
  return std::nullopt;
#endif
}

std::optional<std::vector<std::uint8_t>> aes256gcm_decrypt(
  const std::vector<std::uint8_t>& key32,
  const AeadCiphertext& in,
  std::string* err) {
#ifdef _WIN32
  if (key32.size() != 32) { if (err) *err = "bad key size"; return std::nullopt; }

  BCRYPT_ALG_HANDLE hAlg = nullptr;
  BCRYPT_KEY_HANDLE hKey = nullptr;
  NTSTATUS st = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
  if (st) { if (err) *err = "BCryptOpenAlgorithmProvider failed"; return std::nullopt; }

  st = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
  if (st) { if (err) *err = "BCryptSetProperty failed"; BCryptCloseAlgorithmProvider(hAlg, 0); return std::nullopt; }

  DWORD cbKeyObj = 0, cbData = 0;
  st = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbKeyObj, sizeof(cbKeyObj), &cbData, 0);
  if (st) { if (err) *err = "BCryptGetProperty failed"; BCryptCloseAlgorithmProvider(hAlg, 0); return std::nullopt; }

  std::vector<std::uint8_t> keyObj(cbKeyObj);
  st = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObj.data(), cbKeyObj, (PUCHAR)key32.data(), (ULONG)key32.size(), 0);
  if (st) { if (err) *err = "BCryptGenerateSymmetricKey failed"; BCryptCloseAlgorithmProvider(hAlg, 0); return std::nullopt; }

  std::vector<std::uint8_t> out;
  out.resize(in.ciphertext.size());

  BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;
  BCRYPT_INIT_AUTH_MODE_INFO(info);
  info.pbNonce = const_cast<PUCHAR>(in.nonce.data());
  info.cbNonce = (ULONG)in.nonce.size();
  info.pbTag = const_cast<PUCHAR>(in.tag.data());
  info.cbTag = (ULONG)in.tag.size();

  ULONG cbOut = 0;
  st = BCryptDecrypt(
    hKey,
    (PUCHAR)in.ciphertext.data(),
    (ULONG)in.ciphertext.size(),
    &info,
    nullptr,
    0,
    out.data(),
    (ULONG)out.size(),
    &cbOut,
    0);

  BCryptDestroyKey(hKey);
  BCryptCloseAlgorithmProvider(hAlg, 0);

  if (st) { if (err) *err = "BCryptDecrypt failed"; return std::nullopt; }
  out.resize(cbOut);
  return out;
#else
  if (err) *err = "aead not supported";
  return std::nullopt;
#endif
}

}

