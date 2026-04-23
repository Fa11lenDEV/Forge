#include "forge_format/hash.h"

#include <blake3.h>

#include <fstream>

namespace forge_format::hash {

Digest32 blake3(std::string_view bytes) {
  Digest32 out{};
  blake3_hasher hasher;
  blake3_hasher_init(&hasher);
  blake3_hasher_update(&hasher, bytes.data(), bytes.size());
  blake3_hasher_finalize(&hasher, out.data(), out.size());
  return out;
}

Digest32 blake3_file(const std::string& path, std::string* err) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    if (err) *err = "failed to open file";
    return {};
  }

  blake3_hasher hasher;
  blake3_hasher_init(&hasher);

  std::string buf;
  buf.resize(1 << 20);
  while (f) {
    f.read(buf.data(), static_cast<std::streamsize>(buf.size()));
    auto n = f.gcount();
    if (n > 0) blake3_hasher_update(&hasher, buf.data(), static_cast<size_t>(n));
  }

  if (!f.eof()) {
    if (err) *err = "failed to read file";
    return {};
  }

  Digest32 out{};
  blake3_hasher_finalize(&hasher, out.data(), out.size());
  return out;
}

static char hex_digit(unsigned v) {
  return static_cast<char>(v < 10 ? ('0' + v) : ('a' + (v - 10)));
}

std::string to_hex(const Digest32& d) {
  std::string out;
  out.resize(d.size() * 2);
  for (size_t i = 0; i < d.size(); ++i) {
    auto b = d[i];
    out[i * 2 + 0] = hex_digit((b >> 4) & 0xF);
    out[i * 2 + 1] = hex_digit(b & 0xF);
  }
  return out;
}

}

