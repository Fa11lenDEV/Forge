#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace forge_format::hash {

using Digest32 = std::array<std::uint8_t, 32>;

Digest32 blake3(std::string_view bytes);
Digest32 blake3_file(const std::string& path, std::string* err);
std::string to_hex(const Digest32& d);

}

