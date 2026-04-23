add_executable(forge
  main.cpp
  commands/init.cpp
  commands/add.cpp
  commands/status.cpp
  commands/commit.cpp
  commands/log.cpp
  commands/diff.cpp
  commands/branch.cpp
  commands/switch.cpp
  commands/reset.cpp
  commands/merge.cpp
  commands/rebase.cpp
  commands/tag.cpp
  commands/stash.cpp
  commands/submodule.cpp
  commands/remote.cpp
  commands/clone.cpp
  commands/fetch.cpp
  commands/push.cpp
  commands/pull.cpp
  commands/import_git.cpp
  commands/export_git.cpp
)

target_link_libraries(forge PRIVATE forge_cli forge_core forge_platform)

set_target_properties(forge PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/apps/forge"
)

