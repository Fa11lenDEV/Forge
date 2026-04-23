#include "forge_core/objectstore/objectstore.h"

#include "forge_core/repo/repo.h"
#include "forge_format/hash.h"
#include "forge_format/pack.h"
#include "forge_format/zstd.h"
#include "forge_platform/fs.h"

#include <cstdint>

namespace forge_core::objectstore {

static std::string type_to_str(ObjectType t) {
  switch (t) {
    case ObjectType::Blob: return "blob";
    case ObjectType::Tree: return "tree";
    case ObjectType::Commit: return "commit";
    case ObjectType::Tag: return "tag";
  }
  return "unknown";
}

std::string type_name(ObjectType t) { return type_to_str(t); }

ObjectType parse_type(std::string_view s) {
  if (s == "blob") return ObjectType::Blob;
  if (s == "tree") return ObjectType::Tree;
  if (s == "commit") return ObjectType::Commit;
  if (s == "tag") return ObjectType::Tag;
  return ObjectType::Blob;
}

static std::string encode_header(ObjectType t, size_t size) {
  return type_to_str(t) + " " + std::to_string(size) + "\0";
}

static std::filesystem::path loose_path(const repo::RepoPaths& p, std::string_view hex) {
  auto dir = p.objects_dir / std::string(hex.substr(0, 2));
  auto file = std::string(hex.substr(2));
  return dir / file;
}

ObjectId store_loose(const std::filesystem::path& workdir, ObjectType type, std::string_view bytes, std::string* err) {
  auto paths = repo::make_paths(workdir);
  if (!forge_platform::fs::ensure_dir(paths.objects_dir)) {
    if (err) *err = "objects dir missing";
    return {};
  }

  auto header = encode_header(type, bytes.size());
  std::string canonical;
  canonical.reserve(header.size() + bytes.size());
  canonical.append(header);
  canonical.append(bytes);

  auto d = forge_format::hash::blake3(canonical);
  auto hex = forge_format::hash::to_hex(d);

  auto out_path = loose_path(paths, hex);
  if (!forge_platform::fs::ensure_dir(out_path.parent_path())) {
    if (err) *err = "failed to create objects directory";
    return {};
  }

  if (forge_platform::fs::exists(out_path)) return ObjectId{hex};

  std::string zerr;
  auto comp = forge_format::zstd::compress(canonical, 3, &zerr);
  if (!comp) {
    if (err) *err = "zstd: " + zerr;
    return {};
  }

  if (!forge_platform::fs::write_file_atomic(out_path, *comp)) {
    if (err) *err = "failed to write object";
    return {};
  }

  return ObjectId{hex};
}

static bool parse_header(std::string_view canonical, ObjectType* out_type, size_t* out_size, size_t* out_off) {
  auto nul = canonical.find('\0');
  if (nul == std::string_view::npos) return false;
  auto header = canonical.substr(0, nul);
  auto sp = header.find(' ');
  if (sp == std::string_view::npos) return false;
  auto ts = header.substr(0, sp);
  auto ss = header.substr(sp + 1);
  size_t size = 0;
  for (char c : ss) {
    if (c < '0' || c > '9') return false;
    size = size * 10 + static_cast<size_t>(c - '0');
  }
  *out_type = parse_type(ts);
  *out_size = size;
  *out_off = nul + 1;
  return true;
}

std::optional<StoredObject> load_loose(const std::filesystem::path& workdir, std::string_view hex, std::string* err) {
  auto paths = repo::make_paths(workdir);
  auto p = loose_path(paths, hex);
  auto raw = forge_platform::fs::read_file(p);
  if (!raw) {
    auto packed = forge_format::pack::read_object_from_packs(paths.forge_dir, hex, err);
    if (!packed) {
      if (err && err->empty()) *err = "object not found";
      return std::nullopt;
    }
    raw = packed;
  }

  std::string zerr;
  auto dec = forge_format::zstd::decompress(*raw, &zerr);
  if (!dec) {
    if (err) *err = "zstd: " + zerr;
    return std::nullopt;
  }

  ObjectType t = ObjectType::Blob;
  size_t size = 0;
  size_t off = 0;
  if (!parse_header(*dec, &t, &size, &off)) {
    if (err) *err = "invalid object";
    return std::nullopt;
  }

  if (off + size > dec->size()) {
    if (err) *err = "truncated object";
    return std::nullopt;
  }

  StoredObject o;
  o.type = t;
  o.data.assign(dec->data() + static_cast<std::ptrdiff_t>(off), size);
  return o;
}

}

