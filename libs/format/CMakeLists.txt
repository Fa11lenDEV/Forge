add_library(forge_format STATIC
  src/hash.cpp
  src/zstd.cpp
  src/pack.cpp
)

target_include_directories(forge_format PUBLIC include)

find_package(blake3 CONFIG QUIET)
find_package(unofficial-blake3 CONFIG QUIET)
find_package(zstd CONFIG REQUIRED)

set(_forge_blake3_target "")
foreach(t
  blake3::blake3
  blake3::blake3_static
  blake3
  unofficial::blake3::blake3
)
  if (TARGET ${t})
    set(_forge_blake3_target ${t})
    break()
  endif()
endforeach()

if (_forge_blake3_target STREQUAL "")
  find_path(FORGE_BLAKE3_INCLUDE_DIR blake3.h)
  find_library(FORGE_BLAKE3_LIBRARY NAMES blake3 blake3-static)
  if (FORGE_BLAKE3_INCLUDE_DIR AND FORGE_BLAKE3_LIBRARY)
    add_library(forge_blake3 UNKNOWN IMPORTED)
    set_target_properties(forge_blake3 PROPERTIES
      IMPORTED_LOCATION "${FORGE_BLAKE3_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FORGE_BLAKE3_INCLUDE_DIR}"
    )
    set(_forge_blake3_target forge_blake3)
  else()
    message(FATAL_ERROR "Could not find BLAKE3 via CMake targets nor via find_library/find_path. Ensure vcpkg installed 'blake3' and you are configuring with the vcpkg toolchain.")
  endif()
endif()

target_link_libraries(forge_format PUBLIC forge_platform ${_forge_blake3_target} zstd::libzstd)

