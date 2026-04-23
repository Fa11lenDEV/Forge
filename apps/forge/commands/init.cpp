#include "forge_core/repo/repo.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int init() {
  std::string err;
  auto wd = forge_platform::path::cwd();
  if (!forge_core::repo::init(wd, &err)) {
    std::cerr << "forge init: " << err << "\n";
    return 1;
  }
  std::cout << "Initialized Forge repository in " << (wd / ".forge").string() << "\n";
  return 0;
}

}

