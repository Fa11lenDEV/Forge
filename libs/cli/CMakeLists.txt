add_library(forge_cli STATIC
  src/args.cpp
  src/help.cpp
)

target_include_directories(forge_cli PUBLIC include)

