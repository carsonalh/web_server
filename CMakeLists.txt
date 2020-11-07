cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED TRUE)

project (web_server_library VERSION 0.0.0 LANGUAGES C CXX)

add_library (web_server STATIC
        include/web/http.hpp
        include/web/text.hpp
        include/web/uri.hpp
        src/character_set.cpp
        src/http_request.cpp
        src/http_response.cpp
        src/uri.cpp
)
target_include_directories (web_server PUBLIC include)

if (WIN32)
    target_compile_options (web_server PRIVATE /W4)
    target_link_libraries (web_server PRIVATE ws2_32)
else ()
    target_compile_options (web_server PRIVATE -Werror -Wall)
endif ()

add_subdirectory (demos)

add_subdirectory (third_party/Catch2)

enable_testing ()
add_subdirectory (tests)

