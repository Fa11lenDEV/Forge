#include "forge_core/transfer/frames.h"

#include <cstdint>

namespace forge_core::transfer {

static constexpr std::uint32_t MAX_FRAME_COUNT = 200000;
static constexpr std::uint32_t MAX_PATH_LEN = 4096;
static constexpr std::uint32_t MAX_FRAME_BYTES = 32u * 1024u * 1024u;
static constexpr std::uint64_t MAX_TOTAL_BYTES = 256ull * 1024ull * 1024ull;

static void write_u32(std::string& out, std::uint32_t v) {
  out.push_back(static_cast<char>(v & 0xFF));
  out.push_back(static_cast<char>((v >> 8) & 0xFF));
  out.push_back(static_cast<char>((v >> 16) & 0xFF));
  out.push_back(static_cast<char>((v >> 24) & 0xFF));
}

static std::optional<std::uint32_t> read_u32(std::string_view s, size_t& off) {
  if (off + 4 > s.size()) return std::nullopt;
  std::uint32_t v = 0;
  v |= static_cast<std::uint32_t>(static_cast<unsigned char>(s[off + 0])) << 0;
  v |= static_cast<std::uint32_t>(static_cast<unsigned char>(s[off + 1])) << 8;
  v |= static_cast<std::uint32_t>(static_cast<unsigned char>(s[off + 2])) << 16;
  v |= static_cast<std::uint32_t>(static_cast<unsigned char>(s[off + 3])) << 24;
  off += 4;
  return v;
}

std::string encode_frames(const std::vector<FileEntry>& entries) {
  std::string out;
  write_u32(out, static_cast<std::uint32_t>(entries.size()));
  for (const auto& e : entries) {
    write_u32(out, static_cast<std::uint32_t>(e.rel_path.size()));
    out.append(e.rel_path);
    write_u32(out, static_cast<std::uint32_t>(e.bytes.size()));
    out.append(e.bytes);
  }
  return out;
}

std::optional<std::vector<FileEntry>> decode_frames(std::string_view bytes, std::string* err) {
  size_t off = 0;
  auto nopt = read_u32(bytes, off);
  if (!nopt) {
    if (err) *err = "truncated";
    return std::nullopt;
  }
  if (*nopt > MAX_FRAME_COUNT) {
    if (err) *err = "too many frames";
    return std::nullopt;
  }
  std::vector<FileEntry> out;
  out.reserve(*nopt);
  std::uint64_t total = 0;
  for (std::uint32_t i = 0; i < *nopt; ++i) {
    auto plen = read_u32(bytes, off);
    if (!plen || off + *plen > bytes.size()) return std::nullopt;
    if (*plen > MAX_PATH_LEN) {
      if (err) *err = "path too long";
      return std::nullopt;
    }
    std::string path(bytes.substr(off, *plen));
    off += *plen;
    auto dlen = read_u32(bytes, off);
    if (!dlen || off + *dlen > bytes.size()) return std::nullopt;
    if (*dlen > MAX_FRAME_BYTES) {
      if (err) *err = "frame too large";
      return std::nullopt;
    }
    total += *plen;
    total += *dlen;
    if (total > MAX_TOTAL_BYTES) {
      if (err) *err = "payload too large";
      return std::nullopt;
    }
    std::string data(bytes.substr(off, *dlen));
    off += *dlen;
    out.push_back({std::move(path), std::move(data)});
  }
  return out;
}

}

