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
    CHECK(uri.path() == std::vector<std::string>{ "", "foo", "bar", "" });
    CHECK(uri.parseFromString("https://www.example.com/src/images/"));
    CHECK(uri.path() == std::vector<std::string>{ "", "src", "images", "" });
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
    CHECK(uri.path() == std::vector<std::string>{ "this", "is", "relative", "" });

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
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "absolute", "" });

    CHECK(uri.parseFromString("http:/this/is/absolute"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "absolute" });

    CHECK(uri.parseFromString("http:/this/is/absolute/"));
    CHECK(uri.host() == "");
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "absolute", "" });
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

TEST_CASE("Correctly parses a uri that has an IPv4 address as a host.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("//127.0.0.1/"));
    CHECK(uri.host() == "127.0.0.1");

    CHECK(uri.parseFromString("//255.255.255.255"));
    CHECK(uri.host() == "255.255.255.255");

    CHECK(uri.parseFromString("//0.0.0.0"));
    CHECK(uri.host() == "0.0.0.0");
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

TEST_CASE("Parses a uri with a ipv6 address as a host.") {
    Uri::Uri uri;

    struct TestVector
    {
        std::string uri;
        std::string host;
    };

    const std::vector<TestVector> cases{
        {"https://[::1]", "::1"},
        {"https://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]", "2001:0db8:85a3:0000:0000:8a2e:0370:7334"},
        {"https://[2001:0db8:85a3::8a2e:0370:7334]", "2001:0db8:85a3::8a2e:0370:7334"},
        {"https://[::ffff:192.0.2.128]", "::ffff:192.0.2.128"},
    };

    int i = 0;
    for (auto& c : cases) {
        INFO("Index is " << i++);
        CHECK(uri.parseFromString(c.uri));
        CHECK(uri.host() == c.host);
    }
}

TEST_CASE("Identifies other URI elements with ipv6 as a host.") {
    Uri::Uri uri;

    CHECK(uri.parseFromString("//[::1]/this/is/a/path"));
    CHECK(uri.path() == std::vector<std::string>{ "", "this", "is", "a", "path" });

    CHECK(uri.parseFromString("//[::1]:8080"));
    CHECK(uri.port() == 8080);

    CHECK(uri.parseFromString("//[::7070]#with-ipv6"));
    CHECK(uri.fragment() == "with-ipv6");
}

TEST_CASE("Parses a uri with a bad ipv6 address.") {
    Uri::Uri uri;
    CHECK_FALSE(uri.parseFromString("//[]"));
    CHECK_FALSE(uri.parseFromString("//[::fffg]"));
    CHECK_FALSE(uri.parseFromString("//[::1:0.0.0.256]"));
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

TEST_CASE("Can construct URI-s with query question mark with an empty query.") {
    Uri::Uri uri;

    uri.setQuery("");
    uri.setHost("example.com");

    CHECK(uri.constructString() == "//example.com?");

    uri.clearQuery();
    uri.setHost("example.com");

    CHECK(uri.constructString() == "//example.com");
}

TEST_CASE("Can construct URI-s with fragment pound sign with an empty fragment.") {
    Uri::Uri uri;

    uri.setFragment("");
    uri.setHost("example.com");

    CHECK(uri.constructString() == "//example.com#");

    uri.clearFragment();
    uri.setHost("example.com");

    CHECK(uri.constructString() == "//example.com");
}

TEST_CASE("Compound examples with empty queries and fragments are parsed correctly.") {
    Uri::Uri uri;

    uri.setQuery("");
    uri.setFragment("");
    uri.setHost("www.example.com");

    CHECK(uri.constructString() == "//www.example.com?#");

    uri.setScheme("https");
    CHECK(uri.constructString() == "https://www.example.com?#");
}

TEST_CASE("Paths with trailing empty strings will have a trailing slash in the constructed uri string.") {
    Uri::Uri uri;

    uri.setHost("google.com");
    uri.setPath({
        ""
        });

    CHECK(uri.constructString() == "//google.com/");

    uri.setPath({ "", "foo", "bar" });
    CHECK(uri.constructString() == "//google.com/foo/bar");

    uri.setPath({ "", "foo", "bar", "" });
    CHECK(uri.constructString() == "//google.com/foo/bar/");

    uri.setPath({ "", "a", "" });
    CHECK(uri.constructString() == "//google.com/a/");
}

TEST_CASE("Relative paths are treated as absolute when constructing a string with no other information.") {
    Uri::Uri uri;

    uri.setHost("example.com");
    uri.setPath({ "a", "relative", "path" });
    CHECK(uri.constructString() == "//example.com/a/relative/path");

    uri.setHost("example.com");
    uri.setPath({ "a", "relative", "path", "" });
    CHECK(uri.constructString() == "//example.com/a/relative/path/");
}

TEST_CASE("Resolves basic relative paths.") {
    Uri::Uri uri;

    uri.setPath({ "", "a", "b" });
    uri.resolvePath({ "c" });

    CHECK(uri.path() == std::vector<std::string>{ "", "a", "b", "c" });

    uri.setPath({ "" });
    uri.resolvePath({ "" });

    CHECK(uri.path() == std::vector<std::string>{ "" });

    uri.setPath({ "" });
    uri.resolvePath({ "", "a" });

    CHECK(uri.path() == std::vector<std::string>{ "", "a" });

    uri.setPath({ "", "foo", "bar" });
    uri.resolvePath({ "baz" });

    CHECK(uri.path() == std::vector<std::string>{ "", "foo", "bar", "baz" });
}

TEST_CASE("Uses trailing slash of path to resolve paths.") {
    Uri::Uri uri;

    uri.setPath({ "", "a", "b" });
    uri.resolvePath({ "c", "" });

    CHECK(uri.path() == std::vector<std::string>{ "", "a", "b", "c", "" });

    uri.setPath({ "", "a", "b", "" });
    uri.resolvePath({ "c", "" });

    CHECK(uri.path() == std::vector<std::string>{ "", "a", "b", "c", "" });

    uri.setPath({ "", "a", "b", "" });
    uri.resolvePath({ "c" });

    CHECK(uri.path() == std::vector<std::string>{ "", "a", "b", "c" });

    uri.setPath({ "", "a", "b", "" });
    uri.resolvePath({ "", "c", "" });

    CHECK(uri.path() == std::vector<std::string>{ "", "c", "" });

    uri.setPath({ "" });
    uri.resolvePath({ "foo", "bar" });

    CHECK(uri.path() == std::vector<std::string>{ "", "foo", "bar" });
}

TEST_CASE("Resolve path can use .. notation to go up one level.") {
    Uri::Uri uri;

    uri.setPath({ "", "foo", "bar" });
    uri.resolvePath({ ".." });

    CHECK(uri.path() == std::vector<std::string>{ "", "foo" });

    uri.setPath({ "", "foo", "bar" });
    uri.resolvePath({ "..", "baz" });

    CHECK(uri.path() == std::vector<std::string>{ "", "foo", "baz" });
}

TEST_CASE("Resolve path interprets . as the same \"directory\" the path is in.") {
    Uri::Uri uri;

    uri.setPath({ "", "foo", "bar" });
    uri.resolvePath({ "." });

    CHECK(uri.path() == std::vector<std::string>{ "", "foo", "bar" });

    uri.setPath({ "", "foo", "bar" });
    uri.resolvePath({ ".", "" });

    CHECK(uri.path() == std::vector<std::string>{ "", "foo", "bar", "" });
}

TEST_CASE("Percent encoding does nothing to an empty string.") {
    CHECK(Uri::Uri::percentEncode("") == "");
}

TEST_CASE("Percent encode does not touch string with unreserved characters.") {
    CHECK(Uri::Uri::percentEncode("foo") == "foo");
    CHECK(Uri::Uri::percentEncode("test") == "test");
    CHECK(Uri::Uri::percentEncode("-") == "-");
}

TEST_CASE("Percent encode correctly encodes reserved characters.") {
    CHECK(Uri::Uri::percentEncode(" ") == "%20");
    CHECK(Uri::Uri::percentEncode("@") == "%40");
    CHECK(Uri::Uri::percentEncode("  ") == "%20%20");
}

TEST_CASE("Percent decode correctly decodes an empty string.") {
    CHECK(Uri::Uri::percentDecode("") == "");
}

TEST_CASE("Percent decode does not modify unreserved characters") {
    CHECK(Uri::Uri::percentDecode("foo") == "foo");
    CHECK(Uri::Uri::percentDecode("test") == "test");
    CHECK(Uri::Uri::percentDecode("-") == "-");
}

TEST_CASE("Percent decode correctly decodes reserved characters.") {
    CHECK(Uri::Uri::percentDecode("%20") == " ");
    CHECK(Uri::Uri::percentDecode("%40") == "@");
    CHECK(Uri::Uri::percentDecode("%2B") == "+");
    CHECK(Uri::Uri::percentDecode("%2b") == "+");
    CHECK(Uri::Uri::percentDecode("%2A") == "*");
    CHECK(Uri::Uri::percentDecode("%2a") == "*");
}

TEST_CASE("Correctly percent-encodes characters when constructing a string.") {
    Uri::Uri uri;

    uri.setHost("example.com");
    uri.setPath({ "foo", "bar ", });

    CHECK(uri.constructString() == "//example.com/foo/bar%20");

    uri.setPath({});
    uri.setQuery("this is a test");

    CHECK(uri.constructString() == "//example.com?this%20is%20a%20test");

    uri.clearQuery();
    uri.setFragment("fragment@@");

    CHECK(uri.constructString() == "//example.com#fragment%40%40");

    uri.clearFragment();
}

#include "./catch_main.hpp"

