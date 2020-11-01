cmake_minimum_required (VERSION 3.8)

enable_testing ()

add_executable (uri_test "uri_test.cpp" "catch_main.hpp")
target_link_libraries (uri_test PRIVATE web_server Catch2::Catch2)
add_test (NAME uri_test COMMAND uri_test)

