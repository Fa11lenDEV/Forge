#include "forge_core/index/index.h"

#include "forge_core/objectstore/objectstore.h"
#include "forge_core/repo/repo.h"
#include "forge_format/hash.h"
#include "forge_platform/fs.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace forge_core::index {

static constexpr std::string_view kMagic = "FORGEIDX1";

static std::uint64_t to_ns(std::filesystem::file_time_type t) {
  using namespace std::chrono;
  auto sctp = time_point_cast<nanoseconds>(t - std::filesystem::file_time_type::clock::now() + system_clock::now());
  return static_cast<std::uint64_t>(sctp.time_since_epoch().count());
}

static bool is_ignored_path(const std::filesystem::path& p) {
  for (auto it = p.begin(); it != p.end(); ++it) {
    if (it->string() == ".forge") return true;
  }
  return false;
}

static bool write_u32(std::string& out, std::uint32_t v) {
  out.push_back(static_cast<char>(v & 0xFF));
  out.push_back(static_cast<char>((v >> 8) & 0xFF));
  out.push_back(static_cast<char>((v >> 16) & 0xFF));
  out.push_back(static_cast<char>((v >> 24) & 0xFF));
  return true;
}

static bool write_u64(std::string& out, std::uint64_t v) {
  for (int i = 0; i < 8; ++i) out.push_back(static_cast<char>((v >> (i * 8)) & 0xFF));
  return true;
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

static std::optional<std::uint64_t> read_u64(std::string_view s, size_t& off) {
  if (off + 8 > s.size()) return std::nullopt;
  std::uint64_t v = 0;
  for (int i = 0; i < 8; ++i) {
    v |= static_cast<std::uint64_t>(static_cast<unsigned char>(s[off + i])) << (i * 8);
  }
  off += 8;
  return v;
}

std::optional<Index> load(const std::filesystem::path& workdir, std::string* err) {
  auto paths = repo::make_paths(workdir);
  auto raw = forge_platform::fs::read_file(paths.index_file);
  if (!raw) {
    if (err) *err = "index missing";
    return std::nullopt;
  }

  Index idx;
  if (raw->empty()) return idx;

  std::string_view s(*raw);
  if (s.size() < kMagic.size() || s.substr(0, kMagic.size()) != kMagic) {
    if (err) *err = "invalid index";
    return std::nullopt;
  }

  size_t off = kMagic.size();
  auto nopt = read_u32(s, off);
  if (!nopt) {
    if (err) *err = "truncated index";
    return std::nullopt;
  }
  auto n = *nopt;

  for (std::uint32_t i = 0; i < n; ++i) {
    auto plen = read_u32(s, off);
    auto hlen = read_u32(s, off);
    auto mtime = read_u64(s, off);
    auto size = read_u64(s, off);
    if (!plen || !hlen || !mtime || !size) {
      if (err) *err = "truncated index";
      return std::nullopt;
    }
    if (off + *plen + *hlen > s.size()) {
      if (err) *err = "truncated index";
      return std::nullopt;
    }
    Entry e;
    e.path.assign(s.data() + off, *plen);
    off += *plen;
    e.blob_hex.assign(s.data() + off, *hlen);
    off += *hlen;
    e.mtime_ns = *mtime;
    e.size = *size;
    idx.entries.emplace(e.path, e);
  }

  return idx;
}

bool save(const std::filesystem::path& workdir, const Index& idx, std::string* err) {
  auto paths = repo::make_paths(workdir);
  std::string out;
  out.append(kMagic);
  write_u32(out, static_cast<std::uint32_t>(idx.entries.size()));
  for (const auto& [k, e] : idx.entries) {
    write_u32(out, static_cast<std::uint32_t>(e.path.size()));
    write_u32(out, static_cast<std::uint32_t>(e.blob_hex.size()));
    write_u64(out, e.mtime_ns);
    write_u64(out, e.size);
    out.append(e.path);
    out.append(e.blob_hex);
  }
  if (!forge_platform::fs::write_file_atomic(paths.index_file, out)) {
    if (err) *err = "failed to write index";
    return false;
  }
  return true;
}

static std::optional<std::string> read_file_bytes(const std::filesystem::path& p, std::string* err) {
  auto raw = forge_platform::fs::read_file(p);
  if (!raw) {
    if (err) *err = "failed to read file";
    return std::nullopt;
  }
  return *raw;
}

static bool add_one(const std::filesystem::path& workdir, Index& idx, const std::filesystem::path& p, std::string* err) {
  if (!std::filesystem::exists(p)) {
    if (err) *err = "file does not exist: " + p.string();
    return false;
  }
  if (std::filesystem::is_directory(p)) {
    for (auto& it : std::filesystem::recursive_directory_iterator(p)) {
      if (it.is_directory()) continue;
      if (is_ignored_path(it.path().lexically_relative(workdir))) continue;
      if (!add_one(workdir, idx, it.path(), err)) return false;
    }
    return true;
  }

  auto rel = p.lexically_relative(workdir).generic_string();
  if (rel.empty() || rel[0] == '.') {
    if (is_ignored_path(std::filesystem::path(rel))) return true;
  }

  std::string rerr;
  auto bytes = read_file_bytes(p, &rerr);
  if (!bytes) {
    if (err) *err = rerr;
    return false;
  }

  std::string oerr;
  auto oid = objectstore::store_loose(workdir, objectstore::ObjectType::Blob, *bytes, &oerr);
  if (oid.hex.empty()) {
    if (err) *err = oerr;
    return false;
  }

  Entry e;
  e.path = rel;
  e.blob_hex = oid.hex;
  e.size = static_cast<std::uint64_t>(bytes->size());
  e.mtime_ns = to_ns(std::filesystem::last_write_time(p));
  idx.entries[e.path] = e;
  return true;
}

bool add_paths(const std::filesystem::path& workdir, const std::vector<std::string>& paths, std::string* err) {
  auto maybe = load(workdir, err);
  if (!maybe) return false;
  auto idx = *maybe;

  for (const auto& s : paths) {
    auto p = std::filesystem::path(s);
    if (p.is_relative()) p = workdir / p;
    if (!add_one(workdir, idx, p, err)) return false;
  }

  return save(workdir, idx, err);
}

std::vector<StatusLine> status(const std::filesystem::path& workdir, std::string* err) {
  std::vector<StatusLine> out;
  auto maybe = load(workdir, err);
  if (!maybe) return out;
  auto idx = *maybe;

  std::unordered_map<std::string, bool> seen;
  for (auto& [k, e] : idx.entries) seen.emplace(k, false);

  for (auto& it : std::filesystem::recursive_directory_iterator(workdir)) {
    if (it.is_directory()) continue;
    auto rel = it.path().lexically_relative(workdir).generic_string();
    if (is_ignored_path(std::filesystem::path(rel))) continue;

    auto found = idx.entries.find(rel);
    if (found == idx.entries.end()) {
      out.push_back({"??", rel});
      continue;
    }

    seen[rel] = true;
    auto sz = static_cast<std::uint64_t>(it.file_size());
    auto mt = to_ns(it.last_write_time());
    if (sz != found->second.size || mt != found->second.mtime_ns) {
      out.push_back({" M", rel});
    }
  }

  for (auto& [k, e] : idx.entries) {
    if (!seen[k]) out.push_back({" D", k});
  }

  return out;
}

std::optional<Entry> get_entry(const std::filesystem::path& workdir, const std::string& rel_path, std::string* err) {
  auto maybe = load(workdir, err);
  if (!maybe) return std::nullopt;
  auto it = maybe->entries.find(rel_path);
  if (it == maybe->entries.end()) return std::nullopt;
  return it->second;
}

bool unstage_paths(const std::filesystem::path& workdir, const std::vector<std::string>& paths, std::string* err) {
  auto maybe = load(workdir, err);
  if (!maybe) return false;
  auto idx = *maybe;

  for (const auto& s : paths) {
    auto p = std::filesystem::path(s).generic_string();
    if (p == ".") continue;
    if (!p.empty() && p[0] == '/') p.erase(0, 1);
    idx.entries.erase(p);
  }
  return save(workdir, idx, err);
}

}

