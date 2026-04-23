#include "forge_cli/help.h"

namespace forge_cli {

std::string root_help() {
  return
R"(forge - a fast VCS with `.forge/` repositories

Usage:
  forge <comando> [opções]

Commands:
  init
  add
  status
  commit
  log
  diff
  branch
  switch
  checkout
  reset
  merge
  rebase
  tag
  stash
  submodule
  remote
  clone
  fetch
  push
  pull
  import-git
  export-git
  help
)";
}

std::string command_help(const std::string& cmd) {
  if (cmd == "init") {
    return
R"(Usage:
  forge init
)";
  }
  if (cmd == "add") {
    return
R"(Usage:
  forge add <paths...>
)";
  }
  if (cmd == "status") {
    return
R"(Usage:
  forge status
)";
  }
  if (cmd == "commit") {
    return
R"(Usage:
  forge commit --m=<message>
)";
  }
  if (cmd == "log") {
    return
R"(Usage:
  forge log
)";
  }
  if (cmd == "diff") {
    return
R"(Usage:
  forge diff
)";
  }
  if (cmd == "branch") {
    return
R"(Usage:
  forge branch
  forge branch <name>
)";
  }
  if (cmd == "switch" || cmd == "checkout") {
    return
R"(Usage:
  forge switch <branch>
  forge checkout <branch>
)";
  }
  if (cmd == "reset") {
    return
R"(Usage:
  forge reset [--hard] <commit>
)";
  }
  if (cmd == "merge") {
    return
R"(Usage:
  forge merge <branch> [--m=<message>]
)";
  }
  if (cmd == "rebase") {
    return
R"(Usage:
  forge rebase <upstream-branch> [--hard]
)";
  }
  if (cmd == "tag") {
    return
R"(Usage:
  forge tag <name>
)";
  }
  if (cmd == "stash") {
    return
R"(Usage:
  forge stash push [--m=<msg>]
  forge stash pop
  forge stash list
)";
  }
  if (cmd == "submodule") {
    return
R"(Usage:
  forge submodule add <url> <path>
  forge submodule status
)";
  }
  if (cmd == "remote") {
    return
R"(Usage:
  forge remote add <name> <path>
  forge remote list
)";
  }
  if (cmd == "clone") {
    return
R"(Usage:
  forge clone <path> [dest]
)";
  }
  if (cmd == "fetch") {
    return
R"(Usage:
  forge fetch [remote]
)";
  }
  if (cmd == "push") {
    return
R"(Usage:
  forge push [remote]
)";
  }
  if (cmd == "pull") {
    return
R"(Usage:
  forge pull [remote]
)";
  }
  if (cmd == "import-git") {
    return
R"(Usage:
  forge import-git <repo_git_path> [dest]
)";
  }
  if (cmd == "export-git") {
    return
R"(Usage:
  forge export-git [dest]
)";
  }
  return root_help();
}

}

