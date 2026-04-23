#include "forge_server/handlers.h"

#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

#include <cstdint>
#include <filesystem>
#include <unordered_map>

namespace forge_server {

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

InfoRefsResult info_refs(const std::filesystem::path& repo_dir, std::string* err) {
  InfoRefsResult out;
  if (!forge_core::repo::is_repo(repo_dir)) {
    if (err) *err = "not a forge repo";
    return out;
  }

  auto refs_dir = repo_dir / ".forge" / "refs" / "heads";
  std::error_code ec;
  if (!std::filesystem::exists(refs_dir, ec)) return out;

  for (auto& it : std::filesystem::directory_iterator(refs_dir, ec)) {
    if (!it.is_regular_file()) continue;
    auto raw = forge_platform::fs::read_file(it.path());
    if (!raw) continue;
    out.text.append(it.path().filename().string());
    out.text.push_back('\t');
    out.text.append(*raw);
    if (!out.text.empty() && out.text.back() != '\n') out.text.push_back('\n');
  }
  return out;
}

static bool is_inside_forge(const std::filesystem::path& rel) {
  for (auto it = rel.begin(); it != rel.end(); ++it) {
    if (it->string() == ".forge") return true;
  }
  return false;
}

std::vector<FileEntry> snapshot_forge_dir(const std::filesystem::path& repo_dir, std::string* err) {
  std::vector<FileEntry> out;
  auto forge_dir = repo_dir / ".forge";
  std::error_code ec;
  if (!std::filesystem::exists(forge_dir, ec)) return out;

  for (auto& it : std::filesystem::recursive_directory_iterator(forge_dir, ec)) {
    if (it.is_directory()) continue;
    if (!it.is_regular_file()) continue;
    auto rel = it.path().lexically_relative(repo_dir).generic_string();
    auto raw = forge_platform::fs::read_file(it.path());
    if (!raw) {
      if (err) *err = "failed to read file";
      return {};
    }
    out.push_back({rel, *raw});
  }
  return out;
}

bool apply_snapshot(const std::filesystem::path& repo_dir, const std::vector<FileEntry>& entries, std::string* err) {
  for (const auto& e : entries) {
    auto rel = std::filesystem::path(e.rel_path);
    if (!is_inside_forge(rel)) continue;
    auto out_path = repo_dir / rel;
    if (!forge_platform::fs::ensure_dir(out_path.parent_path())) {
      if (err) *err = "failed to create directory";
      return false;
    }
    if (!forge_platform::fs::write_file_atomic(out_path, e.bytes)) {
      if (err) *err = "failed to write file";
      return false;
    }
  }
  return true;
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
  std::vector<FileEntry> out;
  out.reserve(*nopt);
  for (std::uint32_t i = 0; i < *nopt; ++i) {
    auto plen = read_u32(bytes, off);
    if (!plen || off + *plen > bytes.size()) return std::nullopt;
    std::string path(bytes.substr(off, *plen));
    off += *plen;
    auto dlen = read_u32(bytes, off);
    if (!dlen || off + *dlen > bytes.size()) return std::nullopt;
    std::string data(bytes.substr(off, *dlen));
    off += *dlen;
    out.push_back({std::move(path), std::move(data)});
  }
  return out;
}

}

