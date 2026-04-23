#include "forge_platform/fs.h"

#include <fstream>
#include <random>

namespace forge_platform::fs {

bool exists(const std::filesystem::path& p) {
  return std::filesystem::exists(p);
}

bool ensure_dir(const std::filesystem::path& p) {
  std::error_code ec;
  if (std::filesystem::exists(p, ec)) return std::filesystem::is_directory(p, ec);
  return std::filesystem::create_directories(p, ec);
}

static std::string random_suffix() {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<std::uint64_t> dis;
  auto v = dis(gen);
  return std::to_string(v);
}

bool write_file_atomic(const std::filesystem::path& path, const std::string& data) {
  auto tmp = path;
  tmp += ".tmp.";
  tmp += random_suffix();

  {
    std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(data.data(), static_cast<std::streamsize>(data.size()));
    if (!f) return false;
  }

  std::error_code ec;
  std::filesystem::rename(tmp, path, ec);
  if (!ec) return true;

  std::filesystem::remove(path, ec);
  ec.clear();
  std::filesystem::rename(tmp, path, ec);
  if (ec) {
    std::filesystem::remove(tmp, ec);
    return false;
  }
  return true;
}

std::optional<std::string> read_file(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return std::nullopt;
  f.seekg(0, std::ios::end);
  auto size = f.tellg();
  f.seekg(0, std::ios::beg);
  std::string out;
  out.resize(static_cast<size_t>(size));
  f.read(out.data(), static_cast<std::streamsize>(out.size()));
  if (!f && !out.empty()) return std::nullopt;
  return out;
}

}

