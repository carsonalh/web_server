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

    CHECK(uri.ParseFromString("c++://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetScheme() == "c++");

    CHECK(uri.ParseFromString("a.://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.GetScheme() == "a.");
}

TEST_CASE("Returns false for a bad scheme.") {
    Uri::Uri uri;

    const std::vector<std::string> bad_scheme_uris{
        "://example.com",
        "0://example.com",
        "0a://example.com",
        "++://example.com",
        "b$://example.com",
    };

    for (const auto& u : bad_scheme_uris) {
        CHECK_FALSE(uri.ParseFromString(u));
    }
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
    CHECK(uri.GetPath() == std::vector<std::string>{ "", "foo", "bar" });

    CHECK(uri.ParseFromString("https://www.example.com/baz/foo?query#fragment"));
    CHECK(uri.GetPath() == std::vector<std::string>{ "", "baz", "foo" });

    CHECK(uri.ParseFromString("ftp:/"));
    CHECK(uri.GetPath() == std::vector<std::string>{ "" });

    CHECK(uri.ParseFromString("https://www.example.com//"));
    CHECK(uri.GetPath() == std::vector<std::string>{ "", "" });
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

    CHECK(uri.ParseFromString("../../g"));
    CHECK(uri.GetHost() == "");
    CHECK(uri.GetPath() == std::vector<std::string>{ "..", "..", "g" });
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

    CHECK(uri.ParseFromString("https://example.com:8080/this/is/a/path"));
    CHECK(uri.HasPort());

    CHECK(uri.ParseFromString("//google.com"));
    CHECK_FALSE(uri.HasPort());

    CHECK(uri.ParseFromString("https:this:is:a:uri"));
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

TEST_CASE("Correctly retrieves user info from the uri.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("//john.doe:password@example.com:0"));
    CHECK(uri.GetUserInfo() == "john.doe:password");

    CHECK(uri.ParseFromString("//example_user:example_password@example.com:0"));
    CHECK(uri.GetUserInfo() == "example_user:example_password");
}

TEST_CASE("Correctly identifies whether or not a uri is relative.") {
    Uri::Uri uri;

    CHECK(uri.ParseFromString("foo/bar"));
    CHECK(uri.ContainsRelativePath());

    CHECK(uri.ParseFromString("/not/relative"));
    CHECK_FALSE(uri.ContainsRelativePath());

    CHECK(uri.ParseFromString("../relative"));
    CHECK(uri.ContainsRelativePath());

    CHECK(uri.ParseFromString("..//relative"));
    CHECK(uri.ContainsRelativePath());
}

TEST_CASE("Correctly constructs a URI with a scheme and host") {
    Uri::Uri uri;

    uri.SetScheme("http");
    uri.SetHost("example.com");

    CHECK(uri.ConstructString() == "http://example.com");

    uri.SetScheme("https");
    CHECK(uri.ConstructString() == "https://example.com");
}

TEST_CASE("Correctly constructs a URI with scheme, host, and path") {
    Uri::Uri uri;

    uri.SetScheme("http");
    uri.SetHost("example.com");
    uri.SetPath({
        "",
        "foo",
        "bar",
        });

    CHECK(uri.ConstructString() == "http://example.com/foo/bar");
    
    auto path = uri.GetPath();
    path.push_back("baz");
    uri.SetPath(path);

    CHECK(uri.ConstructString() == "http://example.com/foo/bar/baz");
}

TEST_CASE("Constructs a URI with a host and no scheme") {
    Uri::Uri uri;

    uri.SetHost("example.com");

    CHECK(uri.ConstructString() == "//example.com");
}

TEST_CASE("Constructs a URI with a port.") {
    Uri::Uri uri;

    uri.SetScheme("http");
    uri.SetHost("www.example.com");
    uri.SetHasPort(true);
    uri.SetPort(8080);

    CHECK(uri.ConstructString() == "http://www.example.com:8080");
}

TEST_CASE("Constructs a URI with a query.") {
    Uri::Uri uri;

    uri.SetScheme("http");
    uri.SetHost("www.example.com");
    uri.SetHasPort(true);
    uri.SetPort(8080);
    uri.SetQuery("main_query");

    CHECK(uri.ConstructString() == "http://www.example.com:8080?main_query");
}

TEST_CASE("Constructs a URI with a fragment.") {
    Uri::Uri uri;

    uri.SetScheme("http");
    uri.SetHost("www.example.com");
    uri.SetHasPort(true);
    uri.SetPort(8080);
    uri.SetFragment("my-fragment");

    CHECK(uri.ConstructString() == "http://www.example.com:8080#my-fragment");
}

#include "./catch_main.hpp"

