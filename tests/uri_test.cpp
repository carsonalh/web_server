#include <uri/uri.hpp>

#include <catch2/catch_all.hpp>

TEST_CASE("Returns true for a valid uri.") {
    Uri::Uri uri;
    CHECK(uri.ParseFromString("https://www.example.com/foo/bar?query#fragment"));
}

TEST_CASE("Correctly identifies the scheme of the uri.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetScheme() == "https");

    CHECK(uri.ParseFromString("ftp://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetScheme() == "ftp");
}

TEST_CASE("Correctly identifies the host of the uri.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "www.example.com");

    CHECK(uri.ParseFromString("//www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "www.example.com");

    CHECK(uri.ParseFromString("https://127.0.0.1/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "127.0.0.1");

    CHECK(uri.ParseFromString("//127.0.0.1/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "127.0.0.1");

    CHECK(uri.ParseFromString("https://host(name).com/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "host(name).com");
}

TEST_CASE("Correctly identifies the path of the uri for absolute path.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetPath() == std::vector<std::string>{"", "foo", "bar"});

    CHECK(uri.ParseFromString("https://www.example.com/baz/foo?query#fragment"));
    CHECK(uri.GetPath() == std::vector<std::string>{"", "baz", "foo"});

    CHECK(uri.ParseFromString("ftp:/"));
    CHECK(uri.GetPath() == std::vector<std::string>{""});
}

TEST_CASE("Correctly identifies the path of the uri with a trailing slash.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com/foo/bar/?query#fragment"));
    CHECK(uri.GetPath() == std::vector<std::string>{"", "foo", "bar"});
}

TEST_CASE("Gives empty path when no \"path\" is given at all.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com?query#fragment"));
    CHECK(uri.GetPath() == std::vector<std::string>{});
}

TEST_CASE("Correctly identifies query.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com?query#fragment"));
    CHECK(uri.GetQuery() == "query");

    CHECK(uri.ParseFromString("https://www.example.com?query=value#fragment"));
    CHECK(uri.GetQuery() == "query=value");

    CHECK(uri.ParseFromString("https://www.example.com?query=valueee////@@:#fragment"));
    CHECK(uri.GetQuery() == "query=valueee////@@:");
}

TEST_CASE("Correctly identifies fragment.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com?query#fragment"));
    CHECK(uri.GetFragment() == "fragment");

    CHECK(uri.ParseFromString("https://www.example.com?query#another-fragment"));
    CHECK(uri.GetFragment() == "another-fragment");
}

TEST_CASE("If query and fragment swapped, only identifies fragment.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com#fragment?fake_query"));
    CHECK(uri.GetFragment() == "fragment?fake_query");

    CHECK(uri.ParseFromString("https://www.example.com#?fake_query"));
    CHECK(uri.GetFragment() == "?fake_query");
}

TEST_CASE("Correctly identifies empty query and empty fragment") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("https://www.example.com?#"));
    CHECK(uri.GetFragment() == "");
    CHECK(uri.GetQuery() == "");

    CHECK(uri.ParseFromString("https://www.example.com"));
    CHECK(uri.GetFragment() == "");
    CHECK(uri.GetQuery() == "");
}

TEST_CASE("Identifies relative path.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("this/is/relative"));
    CHECK(uri.GetHost() == "");
    CHECK(uri.GetPath() == std::vector<std::string>{ "this", "is", "relative" });

    CHECK(uri.ParseFromString("this/is/relative/"));
    CHECK(uri.GetHost() == "");
    CHECK(uri.GetPath() == std::vector<std::string>{ "this", "is", "relative" });
}

TEST_CASE("Identifies absolute path correctly.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("/this/is/absolute"));
    CHECK(uri.GetHost() == "");
    CHECK(uri.GetPath() == std::vector<std::string>{ "", "this", "is", "absolute" });

    CHECK(uri.ParseFromString("/this/is/absolute/"));
    CHECK(uri.GetHost() == "");
    CHECK(uri.GetPath() == std::vector<std::string>{ "", "this", "is", "absolute" });

    CHECK(uri.ParseFromString("http:/this/is/absolute"));
    CHECK(uri.GetHost() == "");
    CHECK(uri.GetPath() == std::vector<std::string>{ "", "this", "is", "absolute" });

    CHECK(uri.ParseFromString("http:/this/is/absolute/"));
    CHECK(uri.GetHost() == "");
    CHECK(uri.GetPath() == std::vector<std::string>{ "", "this", "is", "absolute" });
}

TEST_CASE("Identifies empty path correctly.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("//example.com"));
    CHECK(uri.GetHost() == "example.com");
    CHECK(uri.GetPath() == std::vector<std::string>{ });
}

TEST_CASE("Returns false for empty string.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("//example.com"));
    CHECK(uri.GetHost() == "example.com");
    CHECK(uri.GetPath() == std::vector<std::string>{ });
}

TEST_CASE("Correctly identifies if has a port or not.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("//example.com:8080"));
    CHECK(uri.HasPort());

    CHECK(uri.ParseFromString("//google.com"));
    CHECK_FALSE(uri.HasPort());
}

TEST_CASE("Does not parse port out of range.") {
    Uri::Uri uri;

    CHECK_FALSE(uri.ParseFromString("//example.com:65536"));
    CHECK_FALSE(uri.HasPort());

    CHECK(uri.ParseFromString("//example.com:65535"));
    CHECK(uri.HasPort());

    CHECK(uri.ParseFromString("//example.com:0"));
    CHECK(uri.HasPort());
}

TEST_CASE("Correctly retrieves the port from a uri.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("//example.com:0"));
    CHECK(uri.GetPort() == 0);

    CHECK(uri.ParseFromString("//example.com:65535"));
    CHECK(uri.GetPort() == 65535);
}

#include "./catch_main.hpp"

