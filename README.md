<p align="center">
  <img src="images/LogoTxt.png" alt="Nome do Projeto" width="100%">
</p>

# ForgeVCS

A C++ VCS with its own repository format (`.forge/`) and a `forge` CLI.

## Requirements

- CMake 3.23+
- C++20 compiler (MSVC / clang / gcc)
- vcpkg (recommended) with dependencies:
  - `zstd`
  - `blake3`
  - `cpp-httplib`

## Build (Windows, PowerShell)

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j
```

Binary output:

- `build/apps/forge/forge.exe`

## Quick start

```powershell
mkdir demo
cd demo
..\build\apps\forge\forge.exe init
echo hello > a.txt
..\build\apps\forge\forge.exe add a.txt
..\build\apps\forge\forge.exe commit --m="first commit"
..\build\apps\forge\forge.exe log
..\build\apps\forge\forge.exe status
```

## Commands

- `forge help [command]`
- `forge init`
- `forge add <paths...>`
- `forge status`
- `forge config --list`
- `forge config --global user.name "Your Name"`
- `forge config --global user.email "you@example.com"`
- `forge diff`
- `forge commit --m=<message>` (or `--message=<message>`)
- `forge log`
- `forge log --oneline`
- `forge branch [name]`
- `forge branch -d <name>`
- `forge switch <branch>` / `forge checkout <branch>`
- `forge checkout -b <branch>`
- `forge checkout -- <path>`
- `forge reset [--hard] <commit>`
- `forge reset HEAD <path...>`
- `forge merge <branch> [--m=<message>]`
- `forge rebase <upstream-branch> [--hard]`
- `forge tag <name>`
- `forge stash push [--m=<msg>]`
- `forge stash list`
- `forge stash apply`
- `forge stash pop`
- `forge submodule add <url> <path>`
- `forge submodule status`
- `forge remote add <name> <path>`
- `forge remote list`
- `forge clone <path> [dest]`
- `forge fetch [remote] [--token=<token>]`
- `forge push [remote] [--token=<token>]`
- `forge pull [remote]`
- `forge import-git <git_repo_path> [dest]`
- `forge export-git [dest]`
- `forge serve --http=:8080 [--repo=<path>]`
- `forge serve --stdio --repo=<path>`

## Notes

- HTTP remotes are supported via a Forge-native protocol:
  - Start a server: `forge serve --http=:8080 --repo=<repo_path>`
  - Add a remote: `forge remote add origin http://127.0.0.1:8080`
  - Then `forge fetch` / `forge push`
- Authentication:
  - Set `FORGE_TOKEN=<token>` on the server to require `Authorization: Bearer <token>`
  - Provide `--token=<token>` on the client (or set `FORGE_TOKEN` in the client environment)
- SSH stdio server mode exists (`forge serve --stdio`), intended to be used behind an SSH forced-command setup.
- `import-git` / `export-git` are currently **snapshot-based** helpers, not full history conversion.

