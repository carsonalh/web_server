#include "web/http.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("Successfully makes the first response lines.") {
    http::Response response;

    response.clear(); // not necessary here, just for copy/pasting
    response.setVersionMajor(1);
    response.setVersionMinor(1);
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    CHECK(response.constructString() == "HTTP/1.1 200 OK\r\n\r\n");

    response.clear();
    response.setVersionMajor(1);
    response.setVersionMinor(1);
    response.setStatusCode(500);
    response.setReasonPhrase("Internal Server Error");
    CHECK(response.constructString() == "HTTP/1.1 500 Internal Server Error\r\n\r\n");
}

TEST_CASE("Uses 1.1 as the default HTTP version.") {
    http::Response response;

    response.clear();
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    CHECK(response.constructString() == "HTTP/1.1 200 OK\r\n\r\n");
}

TEST_CASE("Can construct response with basic headers.") {
    http::Response response;

    response.clear();
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    response.setHeader("Content-Length", "0");
    response.setHeader("Content-Type", "text/html");
    CHECK(response.constructString() == "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nContent-Type: text/html\r\n\r\n");
}

TEST_CASE("Constructs examples with bodies.") {
    http::Response response;

    response.clear();
    response.setStatusCode(400);
    response.setReasonPhrase("Bad Request");
    response.setHeader("Content-Length", "5");
    response.setHeader("Content-Type", "text/plain");
    response.setBody("error");
    CHECK(response.constructString() == "HTTP/1.1 400 Bad Request\r\nContent-Length: 5\r\nContent-Type: text/plain\r\n\r\nerror");
}

TEST_CASE("Can construct http responses with different given versions.") {
    http::Response response;

    response.clear();
    response.setVersionMajor(1);
    response.setVersionMinor(2);
    response.setStatusCode(400);
    response.setReasonPhrase("Bad Request");
    response.setHeader("Content-Length", "5");
    response.setHeader("Content-Type", "text/plain");
    response.setBody("error");
    CHECK(response.constructString() == "HTTP/1.2 400 Bad Request\r\nContent-Length: 5\r\nContent-Type: text/plain\r\n\r\nerror");
}

#include "./catch_main.hpp"

