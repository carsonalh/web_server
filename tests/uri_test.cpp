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

    CHECK(uri.ParseFromString("https:www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "www.example.com");

    CHECK(uri.ParseFromString("https://127.0.0.1/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "127.0.0.1");

    CHECK(uri.ParseFromString("https:127.0.0.1/foo/bar?query#fragment"));
    CHECK(uri.GetHost() == "127.0.0.1");

    CHECK(uri.ParseFromString("https:host(name).com/foo/bar?query#fragment"));
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

int main(int argc, char **argv)
{
    return Catch::Session().run(argc, argv);
}

