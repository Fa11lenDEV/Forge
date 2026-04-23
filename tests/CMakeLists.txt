add_executable(forge_smoke smoke.cpp)
target_link_libraries(forge_smoke PRIVATE forge_core forge_platform)
add_test(NAME forge_smoke COMMAND forge_smoke)

add_executable(forge_integration integration.cpp)
target_link_libraries(forge_integration PRIVATE forge_core forge_platform)
add_test(NAME forge_integration COMMAND forge_integration)

