#include "forge_core/index/index.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int status() {
  std::string err;
  auto wd = forge_platform::path::cwd();
  auto lines = forge_core::index::status(wd, &err);
  if (!err.empty() && lines.empty()) {
    std::cerr << "forge status: " << err << "\n";
    return 1;
  }

  if (lines.empty()) {
    std::cout << "Nothing to show.\n";
    return 0;
  }

  for (const auto& l : lines) {
    std::cout << l.code << " " << l.path << "\n";
  }
  return 0;
}

}

