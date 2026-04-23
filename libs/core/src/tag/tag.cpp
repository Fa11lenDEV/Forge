#include "forge_core/tag/tag.h"

#include "forge_core/refs/refs.h"

namespace forge_core::tag {

bool create_lightweight(const std::filesystem::path& workdir, const std::string& name, const std::string& target_hex, std::string* err) {
  return forge_core::refs::write_ref(workdir, "refs/tags/" + name, target_hex, err);
}

}

