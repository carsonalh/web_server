#include <uri/uri.hpp>

#include <catch2/catch_all.hpp>

TEST_CASE("Returns true for a valid uri.") {
    Uri::Uri uri;
    CHECK(uri.parseFromString("https://www.example.com/foo/bar?query#fragment"));
}

TEST_CASE("Correctly identifies the scheme of the uri.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.scheme() == "https");

    CHECK(uri.parseFromString("ftp://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.scheme() == "ftp");

    CHECK(uri.parseFromString("c++://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.scheme() == "c++");

    CHECK(uri.parseFromString("a.://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.scheme() == "a.");
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
        CHECK_FALSE(uri.parseFromString(u));
    }
}

TEST_CASE("Correctly identifies the host of the uri.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.host() == "www.example.com");

    CHECK(uri.parseFromString("//www.example.com/foo/bar?query#fragment"));
    CHECK(uri.host() == "www.example.com");

    CHECK(uri.parseFromString("https://127.0.0.1/foo/bar?query#fragment"));
    CHECK(uri.host() == "127.0.0.1");

    CHECK(uri.parseFromString("//127.0.0.1/foo/bar?query#fragment"));
    CHECK(uri.host() == "127.0.0.1");

    CHECK(uri.parseFromString("https://host(name).com/foo/bar?query#fragment"));
    CHECK(uri.host() == "host(name).com");
}

TEST_CASE("Correctly identifies the path of the uri for absolute path.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com/foo/bar?query#fragment"));
    CHECK(uri.path() == std::vector<std::string>{ "", "foo", "bar" });

    CHECK(uri.parseFromString("https://www.example.com/baz/foo?query#fragment"));
    CHECK(uri.path() == std::vector<std::string>{ "", "baz", "foo" });

    CHECK(uri.parseFromString("ftp:/"));
    CHECK(uri.path() == std::vector<std::string>{ "" });

    CHECK(uri.parseFromString("https://www.example.com//"));
    CHECK(uri.path() == std::vector<std::string>{ "", "" });
}

TEST_CASE("Correctly identifies the path of the uri with a trailing slash.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com/foo/bar/?query#fragment"));
    CHECK(uri.path() == std::vector<std::string>{"", "foo", "bar"});
}

TEST_CASE("Gives empty path when no \"path\" is given at all.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com?query#fragment"));
    CHECK(uri.path() == std::vector<std::string>{});
}

TEST_CASE("Correctly identifies query.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com?query#fragment"));
    CHECK(uri.query() == "query");

    CHECK(uri.parseFromString("https://www.example.com?query=value#fragment"));
    CHECK(uri.query() == "query=value");

    CHECK(uri.parseFromString("https://www.example.com?query=valueee////@@:#fragment"));
    CHECK(uri.query() == "query=valueee////@@:");
}

TEST_CASE("Correctly identifies fragment.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com?query#fragment"));
    CHECK(uri.fragment() == "fragment");

    CHECK(uri.parseFromString("https://www.example.com?query#another-fragment"));
    CHECK(uri.fragment() == "another-fragment");
}

TEST_CASE("If query and fragment swapped, only identifies fragment.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com#fragment?fake_query"));
    CHECK(uri.fragment() == "fragment?fake_query");

    CHECK(uri.parseFromString("https://www.example.com#?fake_query"));
    CHECK(uri.fragment() == "?fake_query");
}

TEST_CASE("Correctly identifies empty query and empty fragment") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("https://www.example.com?#"));
    CHECK(uri.fragment() == "");
    CHECK(uri.query() == "");

    CHECK(uri.parseFromString("https://www.example.com"));
    CHECK(uri.fragment() == "");
    CHECK(uri.query() == "");
}

TEST_CASE("Identifies relative path.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("this/is/relative"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "this", "is", "relative" });

    CHECK(uri.parseFromString("this/is/relative/"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "this", "is", "relative" });

    CHECK(uri.parseFromString("../../g"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "..", "..", "g" });
}

TEST_CASE("Identifies absolute path correctly.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("/this/is/absolute"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "absolute" });

    CHECK(uri.parseFromString("/this/is/absolute/"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "absolute" });

    CHECK(uri.parseFromString("http:/this/is/absolute"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "absolute" });

    CHECK(uri.parseFromString("http:/this/is/absolute/"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "absolute" });
}

TEST_CASE("Identifies empty path correctly.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("//example.com"));
    CHECK(uri.host() == "example.com");
    CHECK(uri.path() == std::vector<std::string>{ });
}

TEST_CASE("Returns false for empty string.") {
    Uri::Uri uri;

    CHECK_FALSE(uri.parseFromString(""));
}

TEST_CASE("Correctly identifies IPv4 addresses.") {
    using Uri = Uri::Uri;
    CHECK(Uri::isIpv4String("0.0.0.0"));
    CHECK(Uri::isIpv4String("255.255.255.0"));
    CHECK_FALSE(Uri::isIpv4String("255.255.255.256"));
    CHECK_FALSE(Uri::isIpv4String("255.255.255.2000"));
}

TEST_CASE("Correctly identifies IPv6 addresses.") {
    using Uri = Uri::Uri;
    CHECK(Uri::isIpv6String("::1"));
    CHECK(Uri::isIpv6String("::ffff:1"));

    CHECK_FALSE(Uri::isIpv6String("::1efg"));
    CHECK_FALSE(Uri::isIpv6String(""));

    CHECK(Uri::isIpv6String("::FFFF:1"));
    CHECK(Uri::isIpv6String("::aaaa:AAAA:abab:f099"));
    CHECK(Uri::isIpv6String("2001:0db8:85a3:0000:0000:8a2e:0370:7334"));
    CHECK(Uri::isIpv6String("2001:0db8:85a3::8a2e:0370:7334"));

    CHECK_FALSE(Uri::isIpv6String("2001:0db8:85a3:0000:0000:8a2e:0370:7334:1234"));
    CHECK_FALSE(Uri::isIpv6String("2001accf:0db8:85a3::8a2e:0370:7334"));
    CHECK_FALSE(Uri::isIpv6String(":::0"));
    CHECK_FALSE(Uri::isIpv6String("1::1::1"));
}

TEST_CASE("Correctly parses a uri that has an IPv4 address as a host.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("//127.0.0.1/"));
    CHECK(uri.host() == "127.0.0.1");

    CHECK(uri.parseFromString("//255.255.255.255"));
    CHECK(uri.host() == "255.255.255.255");

    CHECK(uri.parseFromString("//0.0.0.0"));
    CHECK(uri.host() == "0.0.0.0");
}

TEST_CASE("Correctly identifies if has a port or not.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("//example.com:8080"));
    CHECK(uri.hasPort());

    CHECK(uri.parseFromString("https://example.com:8080/this/is/a/path"));
    CHECK(uri.hasPort());

    CHECK(uri.parseFromString("//google.com"));
    CHECK_FALSE(uri.hasPort());

    CHECK(uri.parseFromString("https:this:is:a:uri"));
    CHECK_FALSE(uri.hasPort());
}

TEST_CASE("Does not parse port out of range.") {
    Uri::Uri uri;

    CHECK_FALSE(uri.parseFromString("//example.com:65536"));
    CHECK_FALSE(uri.hasPort());

    CHECK(uri.parseFromString("//example.com:65535"));
    CHECK(uri.hasPort());

    CHECK(uri.parseFromString("//example.com:0"));
    CHECK(uri.hasPort());
}

TEST_CASE("Correctly retrieves the port from a uri.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("//example.com:0"));
    CHECK(uri.port() == 0);

    CHECK(uri.parseFromString("//example.com:65535"));
    CHECK(uri.port() == 65535);
}

TEST_CASE("Correctly retrieves user info from the uri.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("//john.doe:password@example.com:0"));
    CHECK(uri.userInfo() == "john.doe:password");

    CHECK(uri.parseFromString("//example_user:example_password@example.com:0"));
    CHECK(uri.userInfo() == "example_user:example_password");
}

TEST_CASE("Correctly identifies whether or not a uri is relative.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("foo/bar"));
    CHECK(uri.containsRelativePath());

    CHECK(uri.parseFromString("/not/relative"));
    CHECK_FALSE(uri.containsRelativePath());

    CHECK(uri.parseFromString("../relative"));
    CHECK(uri.containsRelativePath());

    CHECK(uri.parseFromString("..//relative"));
    CHECK(uri.containsRelativePath());
}

TEST_CASE("Correctly constructs a URI with a scheme and host") {
    Uri::Uri uri;

    uri.setScheme("http");
    uri.setHost("example.com");

    CHECK(uri.constructString() == "http://example.com");

    uri.setScheme("https");
    CHECK(uri.constructString() == "https://example.com");
}

TEST_CASE("Correctly constructs a URI with scheme, host, and path") {
    Uri::Uri uri;

    uri.setScheme("http");
    uri.setHost("example.com");
    uri.setPath({
        "",
        "foo",
        "bar",
        });

    CHECK(uri.constructString() == "http://example.com/foo/bar");
    
    auto path = uri.path();
    path.push_back("baz");
    uri.setPath(path);

    CHECK(uri.constructString() == "http://example.com/foo/bar/baz");
}

TEST_CASE("Constructs a URI with a host and no scheme") {
    Uri::Uri uri;

    uri.setHost("example.com");

    CHECK(uri.constructString() == "//example.com");
}

TEST_CASE("Constructs a URI with a port.") {
    Uri::Uri uri;

    uri.setScheme("http");
    uri.setHost("www.example.com");
    uri.setHasPort(true);
    uri.setPort(8080);

    CHECK(uri.constructString() == "http://www.example.com:8080");
}

TEST_CASE("Constructs a URI with a query.") {
    Uri::Uri uri;

    uri.setScheme("http");
    uri.setHost("www.example.com");
    uri.setHasPort(true);
    uri.setPort(8080);
    uri.setQuery("main_query");

    CHECK(uri.constructString() == "http://www.example.com:8080?main_query");
}

TEST_CASE("Constructs a URI with a fragment.") {
    Uri::Uri uri;

    uri.setScheme("http");
    uri.setHost("www.example.com");
    uri.setHasPort(true);
    uri.setPort(8080);
    uri.setFragment("my-fragment");

    CHECK(uri.constructString() == "http://www.example.com:8080#my-fragment");
}

#include "./catch_main.hpp"

