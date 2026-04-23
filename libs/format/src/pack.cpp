#include "forge_format/pack.h"

#include "forge_platform/fs.h"

#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace forge_format::pack {

static constexpr std::string_view kPackMagic = "FORGEPK1";

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

static std::string hex_from_loose_path(const std::filesystem::path& objects_dir, const std::filesystem::path& p) {
  auto rel = p.lexically_relative(objects_dir);
  auto it = rel.begin();
  if (it == rel.end()) return {};
  auto d = it->string();
  ++it;
  if (it == rel.end()) return {};
  auto f = it->string();
  return d + f;
}

bool repack_loose_objects(const std::filesystem::path& forge_dir, std::string* err) {
  auto objects_dir = forge_dir / "objects";
  auto packs_dir = forge_dir / "packs";
  if (!forge_platform::fs::ensure_dir(packs_dir)) {
    if (err) *err = "failed to create packs dir";
    return false;
  }

  std::vector<std::pair<std::string, std::filesystem::path>> objs;
  std::error_code ec;
  if (!std::filesystem::exists(objects_dir, ec)) return true;
  for (auto& it : std::filesystem::recursive_directory_iterator(objects_dir, ec)) {
    if (!it.is_regular_file()) continue;
    auto hex = hex_from_loose_path(objects_dir, it.path());
    if (hex.size() < 10) continue;
    objs.emplace_back(std::move(hex), it.path());
  }
  if (objs.empty()) return true;

  std::string pack;
  pack.append(kPackMagic);
  write_u32(pack, static_cast<std::uint32_t>(objs.size()));

  for (const auto& [hex, path] : objs) {
    auto raw = forge_platform::fs::read_file(path);
    if (!raw) {
      if (err) *err = "failed to read loose object";
      return false;
    }
    write_u32(pack, static_cast<std::uint32_t>(hex.size()));
    pack.append(hex);
    write_u32(pack, static_cast<std::uint32_t>(raw->size()));
    pack.append(*raw);
  }

  auto pack_path = packs_dir / "pack.fpk";
  if (!forge_platform::fs::write_file_atomic(pack_path, pack)) {
    if (err) *err = "failed to write pack";
    return false;
  }
  return true;
}

std::optional<std::string> read_object_from_packs(const std::filesystem::path& forge_dir, std::string_view hex, std::string* err) {
  auto pack_path = forge_dir / "packs" / "pack.fpk";
  auto raw = forge_platform::fs::read_file(pack_path);
  if (!raw) return std::nullopt;

  std::string_view s(*raw);
  if (s.size() < kPackMagic.size() || s.substr(0, kPackMagic.size()) != kPackMagic) return std::nullopt;

  size_t off = kPackMagic.size();
  auto nopt = read_u32(s, off);
  if (!nopt) return std::nullopt;
  for (std::uint32_t i = 0; i < *nopt; ++i) {
    auto hlen = read_u32(s, off);
    if (!hlen || off + *hlen > s.size()) return std::nullopt;
    auto h = s.substr(off, *hlen);
    off += *hlen;
    auto dlen = read_u32(s, off);
    if (!dlen || off + *dlen > s.size()) return std::nullopt;
    if (h == hex) return std::string(s.substr(off, *dlen));
    off += *dlen;
  }
  if (err) *err = "object not found in packs";
  return std::nullopt;
}

}

