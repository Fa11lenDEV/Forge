#include "forge_cli/args.h"
#include "forge_cli/help.h"

#include <iostream>

namespace forge_app::commands {
int init();
int add(const forge_cli::ParsedArgs& a);
int status();
int commit(const forge_cli::ParsedArgs& a);
int log();
int diff();
int branch(const forge_cli::ParsedArgs& a);
int sw(const forge_cli::ParsedArgs& a);
int reset(const forge_cli::ParsedArgs& a);
int merge(const forge_cli::ParsedArgs& a);
int rebase(const forge_cli::ParsedArgs& a);
int tag(const forge_cli::ParsedArgs& a);
int stash(const forge_cli::ParsedArgs& a);
int submodule(const forge_cli::ParsedArgs& a);
int remote(const forge_cli::ParsedArgs& a);
int clone(const forge_cli::ParsedArgs& a);
int fetch(const forge_cli::ParsedArgs& a);
int push(const forge_cli::ParsedArgs& a);
int pull(const forge_cli::ParsedArgs& a);
int import_git(const forge_cli::ParsedArgs& a);
int export_git(const forge_cli::ParsedArgs& a);
}

static int run(const forge_cli::ParsedArgs& a) {
  if (a.command.empty() || a.command == "help" || a.has_flag("help")) {
    if (!a.positionals.empty()) {
      std::cout << forge_cli::command_help(a.positionals[0]);
      return 0;
    }
    std::cout << forge_cli::root_help();
    return 0;
  }

  if (a.command == "init") return forge_app::commands::init();
  if (a.command == "add") return forge_app::commands::add(a);
  if (a.command == "status") return forge_app::commands::status();
  if (a.command == "commit") return forge_app::commands::commit(a);
  if (a.command == "log") return forge_app::commands::log();
  if (a.command == "diff") return forge_app::commands::diff();
  if (a.command == "branch") return forge_app::commands::branch(a);
  if (a.command == "switch" || a.command == "checkout") return forge_app::commands::sw(a);
  if (a.command == "reset") return forge_app::commands::reset(a);
  if (a.command == "merge") return forge_app::commands::merge(a);
  if (a.command == "rebase") return forge_app::commands::rebase(a);
  if (a.command == "tag") return forge_app::commands::tag(a);
  if (a.command == "stash") return forge_app::commands::stash(a);
  if (a.command == "submodule") return forge_app::commands::submodule(a);
  if (a.command == "remote") return forge_app::commands::remote(a);
  if (a.command == "clone") return forge_app::commands::clone(a);
  if (a.command == "fetch") return forge_app::commands::fetch(a);
  if (a.command == "push") return forge_app::commands::push(a);
  if (a.command == "pull") return forge_app::commands::pull(a);
  if (a.command == "import-git") return forge_app::commands::import_git(a);
  if (a.command == "export-git") return forge_app::commands::export_git(a);

  std::cerr << "Unknown command: " << a.command << "\n";
  std::cerr << forge_cli::root_help();
  return 2;
}

int main(int argc, char** argv) {
  auto a = forge_cli::parse_args(argc, argv);
  return run(a);
}

