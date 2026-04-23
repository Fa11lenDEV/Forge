#include "forge_cli/args.h"
#include "forge_core/refs/refs.h"
#include "forge_core/tag/tag.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int tag(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge tag: provide a name\n";
    return 2;
  }
  auto name = a.positionals[0];
  std::string err;
  auto wd = forge_platform::path::cwd();
  auto target = forge_core::refs::resolve_head_commit(wd, &err);
  if (!target || target->empty()) {
    std::cerr << "forge tag: no commit\n";
    return 1;
  }
  if (!forge_core::tag::create_lightweight(wd, name, *target, &err)) {
    std::cerr << "forge tag: " << err << "\n";
    return 1;
  }
  return 0;
}

}

