cmake_minimum_required(VERSION 3.23)

project(ForgeVCS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FORGE_ENABLE_TESTS "Build tests" ON)

add_subdirectory(libs/cli)
add_subdirectory(libs/platform)
add_subdirectory(libs/format)
add_subdirectory(libs/core)
add_subdirectory(libs/server)

add_subdirectory(apps/forge)

if (FORGE_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

