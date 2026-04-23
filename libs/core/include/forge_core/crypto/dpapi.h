#pragma once

#include <optional>
#include <string>
#include <vector>

namespace forge_core::crypto::dpapi {

std::optional<std::vector<std::uint8_t>> protect(const std::vector<std::uint8_t>& plaintext, std::string* err);
std::optional<std::vector<std::uint8_t>> unprotect(const std::vector<std::uint8_t>& blob, std::string* err);

}

