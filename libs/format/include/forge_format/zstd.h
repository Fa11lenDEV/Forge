#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace forge_format::zstd {

std::optional<std::string> compress(std::string_view in, int level, std::string* err);
std::optional<std::string> decompress(std::string_view in, std::string* err);

}

