#include "forge_platform/path.h"

namespace forge_platform::path {

std::filesystem::path cwd() {
  return std::filesystem::current_path();
}

}

