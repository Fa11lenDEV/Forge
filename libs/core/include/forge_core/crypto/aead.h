#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace forge_core::crypto::aead {

struct AeadCiphertext {
  std::vector<std::uint8_t> nonce;
  std::vector<std::uint8_t> ciphertext;
  std::vector<std::uint8_t> tag;
};

std::optional<AeadCiphertext> aes256gcm_encrypt(
  const std::vector<std::uint8_t>& key32,
  const std::vector<std::uint8_t>& plaintext,
  std::string* err);

std::optional<std::vector<std::uint8_t>> aes256gcm_decrypt(
  const std::vector<std::uint8_t>& key32,
  const AeadCiphertext& in,
  std::string* err);

}

