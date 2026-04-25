#include "forge_cli/help.h"

namespace forge_cli {

std::string root_help() {
  return
R"(forge - a fast VCS with `.forge/` repositories

Usage:
  forge <command> [options]

Commands:
  init
  add
  status
  config
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
  auth
  clone
  fetch
  push
  pull
  import-git
  export-git
  serve
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
  if (cmd == "config") {
    return
R"(Usage:
  forge config --list
  forge config --global --list
  forge config [--global] <key> <value>

Examples:
  forge config --global user.name "Your Name"
  forge config --global user.email "you@example.com"
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
  forge log [--oneline]
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
  forge branch -d <name>
)";
  }
  if (cmd == "switch" || cmd == "checkout") {
    return
R"(Usage:
  forge switch <branch>
  forge checkout <branch>
  forge checkout -b <branch>
  forge checkout -- <path>
)";
  }
  if (cmd == "reset") {
    return
R"(Usage:
  forge reset [--hard] <commit>
  forge reset HEAD <path...>
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
  forge stash apply
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
  if (cmd == "auth") {
    return
R"(Usage:
  forge auth set-token <remote> <token>
  forge auth show <remote>
  forge auth delete <remote>
)";
  }
  if (cmd == "clone") {
    return
R"(Usage:
  forge clone <source> [dest] [--token=<token>]

Supported sources:
  - Forge local repository path
  - Forge HTTP/HTTPS URL
  - Git local repository path (imports snapshot into Forge)
  - Git HTTP/HTTPS URL (cloned via built-in libgit2, no git.exe)
)";
  }
  if (cmd == "fetch") {
    return
R"(Usage:
  forge fetch [remote] [--token=<token>]
)";
  }
  if (cmd == "push") {
    return
R"(Usage:
  forge push [remote] [--token=<token>]
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
  forge export-git [dest] [--force]
)";
  }
  if (cmd == "serve") {
    return
R"(Usage:
  forge serve --http=:8080 [--repo=<path>] [--public]
  forge serve --https=:8443 --cert=<cert.pem> --key=<key.pem> [--repo=<path>] [--public]
  forge serve --stdio --repo=<path>

Environment:
  FORGE_TOKEN=<token> (optional; enables Bearer auth)
)";
  }
  return root_help();
}

}

