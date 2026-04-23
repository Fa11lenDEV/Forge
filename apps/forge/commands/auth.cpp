#include "forge_cli/args.h"
#include "forge_core/crypto/credentials.h"
#include "forge_platform/path.h"

#include <iostream>

namespace forge_app::commands {

int auth(const forge_cli::ParsedArgs& a) {
  auto sub = a.positionals.empty() ? "" : a.positionals[0];
  auto wd = forge_platform::path::cwd();
  std::string err;

  if (sub == "set-token") {
    if (a.positionals.size() < 3) {
      std::cerr << "forge auth set-token: usage: forge auth set-token <remote> <token>\n";
      return 2;
    }
    if (!forge_core::crypto::credentials::set_token(wd, a.positionals[1], a.positionals[2], &err)) {
      std::cerr << "forge auth: " << err << "\n";
      return 1;
    }
    return 0;
  }

  if (sub == "show") {
    if (a.positionals.size() < 2) {
      std::cerr << "forge auth show: usage: forge auth show <remote>\n";
      return 2;
    }
    auto t = forge_core::crypto::credentials::get_token(wd, a.positionals[1], &err);
    if (!t) {
      std::cerr << "forge auth: token not found\n";
      return 1;
    }
    std::cout << "***\n";
    return 0;
  }

  if (sub == "delete") {
    if (a.positionals.size() < 2) {
      std::cerr << "forge auth delete: usage: forge auth delete <remote>\n";
      return 2;
    }
    if (!forge_core::crypto::credentials::delete_token(wd, a.positionals[1], &err)) {
      std::cerr << "forge auth: " << err << "\n";
      return 1;
    }
    return 0;
  }

  std::cerr << "forge auth: usage:\n";
  std::cerr << "  forge auth set-token <remote> <token>\n";
  std::cerr << "  forge auth show <remote>\n";
  std::cerr << "  forge auth delete <remote>\n";
  return 2;
}

}

