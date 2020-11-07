#include "web/http.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("Http object parses the version.") {
    http::Request http;

    CHECK(
        http.parseFromString(
            "GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1\r\n"
            "\r\n"
        )
    );

    CHECK(http.httpVersionMajor() == 1);
    CHECK(http.httpVersionMinor() == 1);

    CHECK(
        http.parseFromString(
            "GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.0\r\n"
            "\r\n"
        )
    );

    CHECK(http.httpVersionMajor() == 1);
    CHECK(http.httpVersionMinor() == 0);
}

TEST_CASE("Does not parse version for invalid request line.") {
    http::Request http;

    CHECK_FALSE(http.parseFromString("\r\n"));
    CHECK_FALSE(http.parseFromString("GET / NOT_REAL_HTTP/1.1\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/a.a\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1,1\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1a\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.b1\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1 \r\n\r\n"));
    CHECK_FALSE(http.parseFromString("HTTP/1.1 GET /\r\n"));
}

TEST_CASE("Parses versions with more than one digit.") {

    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/2.14\r\n\r\n"));
    CHECK(http.httpVersionMajor() == 2);
    CHECK(http.httpVersionMinor() == 14);

    CHECK(http.parseFromString("GET / HTTP/91.2\r\n\r\n"));
    CHECK(http.httpVersionMajor() == 91);
    CHECK(http.httpVersionMinor() == 2);

}

TEST_CASE("Parses the method correctly.") {
    http::Request http;

    http.parseFromString("GET / HTTP/1.1\r\n\r\n");
    CHECK(http.method() == "GET");

    http.parseFromString("POST / HTTP/1.1\r\n\r\n");
    CHECK(http.method() == "POST");

    http.parseFromString("OPTION * HTTP/1.1\r\n\r\n");
    CHECK(http.method() == "OPTION");
}

TEST_CASE("Invalid method in request line gives error.") {
    http::Request http;

    CHECK_FALSE(http.parseFromString(" GET / HTTP/1.1\r\n\r\n"));
    CHECK_FALSE(http.parseFromString(" GET HTTP/1.1\r\n\r\n"));
}

TEST_CASE("Does not allow double spaces.") {
    http::Request http;

    CHECK_FALSE(http.parseFromString("GET/  HTTP/1.1\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET  /HTTP/1.1\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET  / HTTP/1.1\r\n\r\n"));
}

TEST_CASE("Ensures request line ends with \\r\\n sequence.") {
    http::Request http;

    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1ab"));
}

TEST_CASE("Ends header block with \\r\\n sequence.") {
    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/1.1\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\n"));
}

TEST_CASE("Is able to parse the request uri.") {
    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/1.1\r\n\r\n"));
    CHECK(http.uri() == "/");

    CHECK(http.parseFromString("OPTIONS * HTTP/1.1\r\n\r\n"));
    CHECK(http.uri() == "*");

    CHECK(http.parseFromString("GET http://example.com/?some_result HTTP/1.1\r\n\r\n"));
    CHECK(http.uri() == "http://example.com/?some_result");
}

TEST_CASE("Is able to parse a header.") {
    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\r\nUser-Agent: X\r\n\r\n"));
    CHECK(http.hasHeader("Content-Type"));
    CHECK(http.header("Content-Type") == "text/html");
    CHECK(http.hasHeader("User-Agent"));
    CHECK(http.header("User-Agent") == "X");

    CHECK_FALSE(http.hasHeader("Non-Existant"));

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/plain\r\n\r\n"));
    CHECK(http.hasHeader("Content-Type"));
    CHECK(http.header("Content-Type") == "text/plain");

    CHECK_FALSE(http.hasHeader("User-Agent"));

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type:text/plain\r\n\r\n"));
    CHECK(http.hasHeader("Content-Type"));
    CHECK(http.header("Content-Type") == "text/plain");
}

TEST_CASE("Does not parse an incorrect header.") {
    http::Request http;

    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nInvalid-Header :Value\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nInvalid-Header\t:Value\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\n Also-Invalid:Value\r\n\r\n"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nInvalid,Header:Value\r\n\r\n"));
}

TEST_CASE("Does not count trailing or preceding whitespace for header values.") {
    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html \r\n\r\n"));
    CHECK(http.header("Content-Type") == "text/html");

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type:\r\n\r\n"));
    CHECK(http.header("Content-Type") == "");

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type:  \r\n\r\n"));
    CHECK(http.header("Content-Type") == "");

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type:\t \r\n\r\n"));
    CHECK(http.header("Content-Type") == "");
}

TEST_CASE("Header names are case insensitive.") {
    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\r\n\r\n"));
    CHECK(http.hasHeader("content-type"));
    CHECK(http.hasHeader("CONTENT-TYPE"));

    CHECK(http.parseFromString("GET / HTTP/1.1\r\ncontent-type: text/html\r\n\r\n"));
    CHECK(http.hasHeader("Content-Type"));
    CHECK(http.hasHeader("CONTENT-TYPE"));
}

TEST_CASE("Correctly parses a simple body from a http request.") {
    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/json\r\n\r\n{\"key\": \"value\"}\r\n"));
    CHECK(http.hasBody());
    CHECK(http.body() == "{\"key\": \"value\"}\r\n");

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\r\n\r\n<h1>this is my title</h1>\r\n"));
    CHECK(http.hasBody());
    CHECK(http.body() == "<h1>this is my title</h1>\r\n");
}

TEST_CASE("Identifies when requests do not have a body.") {
    http::Request http;

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nUser-Agent: X\r\n\r\n"));
    CHECK_FALSE(http.hasBody());

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\r\n\r\n"));
    CHECK_FALSE(http.hasBody());

    CHECK(http.parseFromString("GET / HTTP/1.1\r\nContent-Type:text/html\r\n\r\n"));
    CHECK_FALSE(http.hasBody());
}

TEST_CASE("Returns false when the header block is not followed by the CRLF sequence.") {
    http::Request http;

    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nUser-Agent: X\r\n"));

    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\r\n<h1>this is my title</h1>\r\n"));
    CHECK_FALSE(http.hasBody());

    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\r\n<h1>this is my title</h1>"));
    CHECK_FALSE(http.hasBody());

    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\r\n\r"));
    CHECK_FALSE(http.parseFromString("GET / HTTP/1.1\r\nContent-Type: text/html\rn\rn"));
}

#include "./catch_main.hpp"

