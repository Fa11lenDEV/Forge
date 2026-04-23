#include "forge_core/repo/repo.h"

#include <cassert>
#include <filesystem>
#include <string>

int main() {
  auto tmp = std::filesystem::temp_directory_path() / "forge_smoke_repo";
  std::error_code ec;
  std::filesystem::remove_all(tmp, ec);
  std::filesystem::create_directories(tmp, ec);

  std::string err;
  auto ok = forge_core::repo::init(tmp, &err);
  assert(ok);
  assert(forge_core::repo::is_repo(tmp));
  return 0;
}

