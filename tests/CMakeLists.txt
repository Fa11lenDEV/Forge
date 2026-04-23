add_executable(forge_smoke smoke.cpp)
target_link_libraries(forge_smoke PRIVATE forge_core forge_platform)
add_test(NAME forge_smoke COMMAND forge_smoke)

add_executable(forge_integration integration.cpp)
target_link_libraries(forge_integration PRIVATE forge_core forge_platform)
add_test(NAME forge_integration COMMAND forge_integration)

add_executable(forge_remote_http remote_http.cpp)
target_link_libraries(forge_remote_http PRIVATE forge_core forge_platform forge_server)
add_test(NAME forge_remote_http COMMAND forge_remote_http)

add_executable(forge_security security.cpp)
target_link_libraries(forge_security PRIVATE forge_core forge_platform)
add_test(NAME forge_security COMMAND forge_security)

