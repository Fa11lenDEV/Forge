#include "forge_cli/args.h"
#include "forge_core/commit/commit.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int commit(const forge_cli::ParsedArgs& a) {
  auto msg = a.option("m");
  if (msg.empty()) msg = a.option("message");
  if (msg.empty()) {
    std::cerr << "forge commit: use --m=<msg> (or --message=<msg>)\n";
    return 2;
  }

  std::string err;
  auto wd = forge_platform::path::cwd();
  auto res = forge_core::commit::create_commit_from_index(wd, msg, &err);
  if (res.commit_hex.empty()) {
    std::cerr << "forge commit: " << err << "\n";
    return 1;
  }
  std::cout << res.commit_hex << "\n";
  return 0;
}

}

