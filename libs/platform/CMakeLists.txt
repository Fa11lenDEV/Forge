add_library(forge_platform STATIC
  src/fs.cpp
  src/path.cpp
)

target_include_directories(forge_platform PUBLIC include)

