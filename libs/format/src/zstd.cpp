#include "forge_format/zstd.h"

#include <zstd.h>

namespace forge_format::zstd {

std::optional<std::string> compress(std::string_view in, int level, std::string* err) {
  auto bound = ZSTD_compressBound(in.size());
  std::string out;
  out.resize(bound);

  auto n = ZSTD_compress(out.data(), out.size(), in.data(), in.size(), level);
  if (ZSTD_isError(n)) {
    if (err) *err = ZSTD_getErrorName(n);
    return std::nullopt;
  }

  out.resize(n);
  return out;
}

std::optional<std::string> decompress(std::string_view in, std::string* err) {
  auto sz = ZSTD_getFrameContentSize(in.data(), in.size());
  if (sz == ZSTD_CONTENTSIZE_ERROR || sz == ZSTD_CONTENTSIZE_UNKNOWN) {
    if (err) *err = "unknown size";
    return std::nullopt;
  }

  std::string out;
  out.resize(static_cast<size_t>(sz));
  auto n = ZSTD_decompress(out.data(), out.size(), in.data(), in.size());
  if (ZSTD_isError(n)) {
    if (err) *err = ZSTD_getErrorName(n);
    return std::nullopt;
  }
  out.resize(n);
  return out;
}

}

