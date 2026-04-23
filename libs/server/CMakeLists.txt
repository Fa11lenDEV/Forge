add_library(forge_server STATIC
  src/handlers.cpp
  src/api.cpp
)

target_include_directories(forge_server PUBLIC include)
target_link_libraries(forge_server PUBLIC forge_core forge_platform)

find_package(httplib CONFIG REQUIRED)
target_link_libraries(forge_server PUBLIC httplib::httplib)

find_package(OpenSSL REQUIRED)
target_link_libraries(forge_server PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_compile_definitions(forge_server PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)

