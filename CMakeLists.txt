cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED TRUE)

project (web_server_library VERSION 0.0.0 LANGUAGES C CXX)

add_subdirectory ("third_party/Catch2")

add_library (web_server STATIC
        "include/uri/uri.hpp"
        "src/uri.cpp"
)
target_include_directories (web_server PUBLIC "include")

enable_testing ()

add_subdirectory ("tests")

