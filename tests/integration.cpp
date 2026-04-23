#include "forge_core/checkout/checkout.h"
#include "forge_core/commit/commit.h"
#include "forge_core/index/index.h"
#include "forge_core/refs/refs.h"
#include "forge_core/repo/repo.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

static void write_file(const std::filesystem::path& p, const std::string& s) {
  std::error_code ec;
  std::filesystem::create_directories(p.parent_path(), ec);
  std::ofstream f(p, std::ios::binary | std::ios::trunc);
  f << s;
}

int main() {
  auto tmp = std::filesystem::temp_directory_path() / "forge_integration_repo";
  std::error_code ec;
  std::filesystem::remove_all(tmp, ec);
  std::filesystem::create_directories(tmp, ec);

  std::string err;
  assert(forge_core::repo::init(tmp, &err));

  write_file(tmp / "a.txt", "hello\n");
  assert(forge_core::index::add_paths(tmp, {"a.txt"}, &err));
  auto c1 = forge_core::commit::create_commit_from_index(tmp, "c1", &err);
  assert(!c1.commit_hex.empty());

  write_file(tmp / "a.txt", "hello2\n");
  assert(forge_core::index::add_paths(tmp, {"a.txt"}, &err));
  auto c2 = forge_core::commit::create_commit_from_index(tmp, "c2", &err);
  assert(!c2.commit_hex.empty());

  assert(forge_core::checkout::checkout_commit(tmp, c1.commit_hex, &err));
  auto msg = forge_core::commit::read_commit_message(tmp, c1.commit_hex, &err);
  assert(msg && *msg == "c1");

  return 0;
}

