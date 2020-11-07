cmake_minimum_required (VERSION 3.8)

enable_testing ()

add_executable (uri_test
        uri_test.cpp
        catch_main.hpp
)
target_link_libraries (uri_test
        PRIVATE
            web_server
            Catch2::Catch2
)
add_test (NAME UriTest COMMAND uri_test)

add_executable (http_request_test
        http_request_test.cpp
        catch_main.hpp
)
target_link_libraries (http_request_test
        PRIVATE
            web_server
            Catch2::Catch2
)
add_test (NAME HttpRequestTest COMMAND http_request_test)

add_executable (http_response_test
        http_response_test.cpp
        catch_main.hpp
)
target_link_libraries (http_response_test
        PRIVATE
            web_server
            Catch2::Catch2
)
add_test (NAME HttpResponseTest COMMAND http_response_test)
