cmake_minimum_required (VERSION 3.8)

add_executable (WebServerMain
        main.cpp
)
target_link_libraries (WebServerMain PRIVATE web_server)
target_compile_definitions (WebServerMain PUBLIC WEB_DEFINE_MAIN)

