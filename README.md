# ForgeVCS

A C++ VCS with its own repository format (`.forge/`) and a `forge` CLI.

## Requirements

- CMake 3.23+
- C++20 compiler (MSVC / clang / gcc)
- vcpkg (recommended) with dependencies:
  - `zstd`
  - `blake3`

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
- `forge diff`
- `forge commit --m=<message>` (or `--message=<message>`)
- `forge log`
- `forge branch [name]`
- `forge switch <branch>` / `forge checkout <branch>`
- `forge reset [--hard] <commit>`
- `forge merge <branch> [--m=<message>]`
- `forge rebase <upstream-branch> [--hard]`
- `forge tag <name>`
- `forge stash push [--m=<msg>]`
- `forge stash list`
- `forge stash pop`
- `forge submodule add <url> <path>`
- `forge submodule status`
- `forge remote add <name> <path>`
- `forge remote list`
- `forge clone <path> [dest]`
- `forge fetch [remote]`
- `forge push [remote]`
- `forge pull [remote]`
- `forge import-git <git_repo_path> [dest]`
- `forge export-git [dest]`

## Notes

- Remotes are currently **filesystem-based** paths (no HTTP/SSH yet).
- `import-git` / `export-git` are currently **snapshot-based** helpers, not full history conversion.

