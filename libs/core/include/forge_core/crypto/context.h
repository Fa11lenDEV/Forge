#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace forge_core::crypto {

struct CryptoContext {
  bool enabled = false;
  std::vector<std::uint8_t> master_key32;
};

std::optional<CryptoContext> load_or_create(const std::filesystem::path& workdir, std::string* err);

std::optional<std::string> encrypt_bytes(const CryptoContext& ctx, const std::string& plaintext, std::string* err);
std::optional<std::string> decrypt_bytes(const CryptoContext& ctx, const std::string& blob, std::string* err);

}

