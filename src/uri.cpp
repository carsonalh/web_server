#include "uri/uri.hpp"

#include <cctype>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <regex>
#include <memory>

namespace Uri {

    /** Implementation details of the Uri::Uri class. */
    struct Uri::Impl
    {
    public:
        /** The maximum amount of non-colon segments in an IPv6 string. */
        const static size_t IPV6_MAX_SEGMENTS = 8;
        /** The maximum amount of hex characters in an IPv6 string. */
        const static size_t IPV6_MAX_HEX_LENGTH = 4;

        /**
         * Helper function that verifies the colon (:) segments in an IPv6
         * address. Makes sure there is only one double colon segment, and that
         * there are not too many segments in string altogether.
         */
        static bool ipv6VerifyColonSegments(const std::string& string);

        /**
         * Makes sure that all the non-colon characters in the string are hex
         * characters, and there is no non-colon segment that is longer than 4
         * characters.
         */
        static bool ipv6VerifyDigitSegments(const std::string& string);

        /**
         * A helper function which, starting from the end index, works
         * backwards to see if the text immediately behind it is an 8-bit
         * unsigned integer written in decimal form.
         * @param[in] string
         * @param[in] end
         *      The index immediately after the search should take place.
         * @returns
         *      Whether the search found a valid integer from 0 to 255 or not.
         */
        static bool verify8bitDecimalFromEnd(const std::string& string, unsigned end);

    public:
        std::string                 scheme;
        std::string                 userInfo;
        std::string                 host;
        std::vector<std::string>    path;
        std::string                 query;
        bool                        hasQuery = false;
        std::string                 fragment;
        bool                        hasFragment = false;
        uint16_t                    port = 0;
        bool                        hasPort = false;

    public:
        /** Clears all the member data of the implementation. */
        void clear()
        {
            scheme.clear();
            userInfo.clear();
            host.clear();
            path.clear();
            query.clear();
            fragment.clear();
            hasQuery = false;
            hasFragment = false;
            hasPort = false;
            port = 0;
        }

    };

    bool Uri::Impl::ipv6VerifyColonSegments(const std::string& string)
    {
        unsigned numSegments = 1;
        unsigned numSingleColons = 0;
        unsigned numDoubleColons = 0;
        unsigned currentColonRun = 0;
        unsigned numDots = 0;

        for (int i = 0; i < string.size(); ++i) {
            if (string[i] == ':') {
                if (currentColonRun > 1) {
                    return false;
                }
                else if (currentColonRun == 1) {
                    if (numDoubleColons > 0)
                        return false;
                    ++numDoubleColons;
                }
                ++currentColonRun;
            }
            else if (string[i] == '.') {
                ++numDots;
            }
            else {
                if (currentColonRun > 0) {
                    ++numSingleColons;
                    ++numSegments;
                }
                currentColonRun = 0;
            }
        }

        const size_t IPV4_NUM_DOTS = 3;
        if (numDots > 0 && numDots != IPV4_NUM_DOTS) return false;

        return numSegments <= IPV6_MAX_SEGMENTS;
    }

    bool Uri::Impl::ipv6VerifyDigitSegments(const std::string& string)
    {
        int currentRun = 0;
        bool hasIpv4 = false;

        for (unsigned i = 0; i < string.size(); ++i) {
            auto c = string[i];
            if (std::isxdigit(c)) {
                if (++currentRun > IPV6_MAX_HEX_LENGTH) {
                    return false;
                }
            }
            else if (c != ':') {
                if (c != '.')  {
                    return false;
                }
                else {
                    currentRun = 0;
                    hasIpv4 = true;
                    if (!verify8bitDecimalFromEnd(string, i))
                        return false;
                }
            }
            else {
                currentRun = 0;
            }
        }

        if (hasIpv4) {
            if (!verify8bitDecimalFromEnd(string, string.size()))
                return false;
        }

        return true;
    }

    bool Uri::Impl::verify8bitDecimalFromEnd(const std::string& string, unsigned end)
    {
        auto j = end;
        int16_t ipv4Segment = 0;
        unsigned powerOfTen = 1;
        while (std::isdigit(string[--j])) {
            ipv4Segment += powerOfTen * (string[j] - '0');
            powerOfTen *= 10;
            if (ipv4Segment & ~0x00ff)
                return false;
        }

        return true;
    }

    bool Uri::isIpv4String(const std::string& string)
    {
        if (string.empty())
            return false;

        bool hasParsedNumber = false;
        uint16_t currentNumber = 0;

        for (int i = 0; i < string.size(); ++i) {
            if (string[i] == '.') {
                if (!hasParsedNumber) {
                    return false;
                }
                else {
                    hasParsedNumber = false;
                    currentNumber = 0;
                    continue;
                }
            }
            else if (std::isdigit(string[i])) {
                hasParsedNumber = true;
                currentNumber *= 10;
                currentNumber += string[i] - '0';
                if (currentNumber & ~0x00ff)
                    return false;
            }
            else {
                return false;
            }
        }

        return hasParsedNumber;
    }

    bool Uri::isIpv6String(const std::string& string)
    {
        if (string.empty()) return false;

        if (!Impl::ipv6VerifyColonSegments(string)) return false;
        if (!Impl::ipv6VerifyDigitSegments(string)) return false;

        return true;
    }

    std::string Uri::percentEncode(std::string_view string, const CharacterSet& allowedCharacters)
    {
        std::ostringstream out;

        for (char c : string) {
            if (allowedCharacters.contains(c)) {
                out << c;
            }
            else {
                static char percentEncode[4] = { '%', 0, 0, 0 };
                std::snprintf(&percentEncode[1], 3, "%2x", c);
                out << percentEncode;
            }
        }

        return out.str();
    }

    std::string Uri::percentDecode(std::string_view string)
    {
        std::ostringstream out;

        for (int i = 0; i < string.size(); ++i) {
            char c = string[i];

            if (c == '%') {
                static char buffer[3] = { 0 };
                int decoded;
                std::memcpy(buffer, string.data() + i + 1, sizeof buffer - sizeof buffer[0]);
                std::sscanf(buffer, "%2X", &decoded);
                out << (char)decoded;
                i += 2; // Advance i past the two characters that were just read
            }
            else {
                out << c;
            }
        }

        return out.str();
    }

    Uri::Uri()
        : m_Impl{ std::make_unique<Uri::Impl>() }
    {
    }

    Uri::~Uri()
    {
    }

    bool Uri::parseFromString(std::string string)
    {
        if (string.empty()) {
            return false;
        }

        std::smatch searchResults;

        m_Impl->clear();

        auto search = [&string, &searchResults](const std::regex& pattern, int resultIndex)
                -> std::string {
            auto found = std::regex_search(string, searchResults, pattern);

            if (found) {
                auto& result = searchResults[resultIndex].str();
                string = searchResults.suffix().str();
                return result;
            }
            else {
                return "";
            }
        };

        if (std::regex_search(string, searchResults, std::regex("^([a-zA-Z][a-zA-Z0-9+\\-.]*):"))) {
            m_Impl->scheme = searchResults[1].str();
            string = searchResults.suffix().str();
        }
        else if (std::regex_search(string, std::regex("^.*://"))) {
            return false;
        }

        {
            std::regex userInfoAndHostPattern{
                "^:?//((([a-zA-Z0-9\\-._~:!$&'\\(\\)*+,;=]|%[0-9a-fA-F]{2})*)@)?"
                "(([a-zA-Z0-9\\-._~]|%[a-fA-F0-9]{2}|[!$&'()*+,;=])+)?"
            };

            if (std::regex_search(string, searchResults, userInfoAndHostPattern)) {
                m_Impl->userInfo = searchResults[2].str();
                m_Impl->host = searchResults[4].str();
                string = searchResults.suffix().str();
            }

            //ipv6 support
            if (m_Impl->host == "") {
                std::regex ipv6HostPattern{"(\\[[a-fA-F0-9:.]+\\])"};
                std::regex badIpv6HostPattern{"(\\[.*\\])"};

                if (std::regex_search(string, searchResults, ipv6HostPattern)) {
                    auto& ipv6RawHost = searchResults[1].str();

                    auto& innerHost = ipv6RawHost.substr(1, ipv6RawHost.size() - 2);
                    if (isIpv6String(innerHost)) {
                        m_Impl->host = innerHost;
                    }
                    else {
                        return false;
                    }

                    string = searchResults.suffix().str();
                }
                else if (std::regex_search(string, badIpv6HostPattern)) {
                    return false;
                }
            }
        }

        std::regex portPattern{ "^:([0-9]{0,5})" };

        {
            const auto& port = search(portPattern, 1);
            m_Impl->hasPort = !port.empty();

            uint32_t portNumber = 0;
            const uint32_t portMask = 0x0000ffff;

            for (auto c : port) {
                // The regex has already guaranteed that c will be numeric
                portNumber *= 10;
                portNumber += c - '0';

                if (portNumber & ~portMask) {
                    m_Impl->hasPort = false;
                    return false;
                }
            }

            m_Impl->port = portNumber;
        }

        std::regex pathPattern{
            "^("
                "/?(([a-zA-Z0-9\\-._~!$&'()*+,;=]|%[a-fA-F0-9]{2})*/)*"
                "([a-zA-Z0-9\\-._~!$&'()*+,;=]|%[a-fA-F0-9]{2})*"
            ")"
        };

        {
            const auto& result = search(pathPattern, 1);

            if (result.size() > 0) {
                for (int i = 0; i < result.size(); ++i) {
                    auto end = i;

                    while (end < result.size() && result[end] != '/') {
                        ++end;
                    }

                    m_Impl->path.push_back(result.substr(i, end - i));

                    i = end;
                }

                if (result[result.size() - 1] == '/' && !m_Impl->path[m_Impl->path.size() - 1].empty()) {
                    m_Impl->path.push_back("");
                }
            }
        }

        std::regex queryPattern{ "^\\?(([a-zA-Z0-9\\-._~!$&'()*+,;=/?:@]|%[a-fA-F0-9]{2})*)" };
        m_Impl->query = search(queryPattern, 1);

        std::regex fragmentPattern{ "^#(([a-zA-Z0-9\\-._~!$&'()*+,;=/?:@]|%[a-fA-F0-9]{2})*)" };
        m_Impl->fragment = search(fragmentPattern, 1);

        if (m_Impl->fragment.size() > 0) {
            if (searchResults.suffix().str().size() > 0) {
                m_Impl->clear();
                return false;
            }
        }

        return true;
    }

    const std::string& Uri::scheme() const
    {
        return m_Impl->scheme;
    }

    const std::string& Uri::userInfo() const
    {
        return m_Impl->userInfo;
    }

    const std::string& Uri::host() const
    {
        return m_Impl->host;
    }

    const std::vector<std::string>& Uri::path() const
    {
        return m_Impl->path;
    }

    const std::string& Uri::query() const
    {
        return m_Impl->query;
    }

    const std::string& Uri::fragment() const
    {
        return m_Impl->fragment;
    }

    bool Uri::hasPort() const
    {
        return m_Impl->hasPort;
    }

    uint16_t Uri::port() const
    {
        return m_Impl->port;
    }

    bool Uri::containsRelativePath() const
    {
        return m_Impl->path.size() && !m_Impl->path[0].empty();
    }

    std::string Uri::constructString() const
    {
        std::ostringstream outString, pathString;

        if (m_Impl->path.size() == 1 && m_Impl->path[0].empty()) {
            pathString << '/';
        }
        else {
            unsigned index = 0;
            for (const auto& segment : m_Impl->path) {
                if (!segment.empty() || index != 0)
                    pathString << '/' << segment;
                ++index;
            }
        }

        if (m_Impl->scheme.size() > 0) {
            outString << m_Impl->scheme << ':';
        }

        outString << "//" << m_Impl->host;

        if (m_Impl->hasPort) {
            outString << ':' << std::to_string(m_Impl->port);
        }

        outString << pathString.str();

        if (m_Impl->hasQuery) {
            outString << '?' + m_Impl->query;
        }

        if (m_Impl->hasFragment) {
            outString << '#' + m_Impl->fragment;
        }

        return outString.str();
    }

    void Uri::resolvePath(const std::vector<std::string>& otherPath)
    {
        auto& path = m_Impl->path;

        if (otherPath.size() > 0 && otherPath[0].empty()) {
            path = otherPath;
            return;
        }

        if (path.size() != 1 && path[path.size() - 1].empty()) {
            path.pop_back();
        }

        for (const auto& p : otherPath) {
            if (p == "..")
                path.pop_back();
            else if (p == ".")
                continue;
            else
                path.push_back(p);
        }
    }

    void Uri::setScheme(const std::string& scheme)
    {
        m_Impl->scheme = scheme;
    }

    void Uri::setUserInfo(const std::string& userInfo)
    {
        m_Impl->userInfo = userInfo;
    }

    void Uri::setHost(const std::string& host)
    {
        m_Impl->host = host;
    }

    void Uri::setPath(const std::vector<std::string>& path)
    {
        m_Impl->path = path;
    }

    void Uri::setQuery(const std::string& query)
    {
        m_Impl->query = query;
        m_Impl->hasQuery = true;
    }

    void Uri::clearQuery()
    {
        m_Impl->hasQuery = false;
    }

    void Uri::setFragment(const std::string& fragment)
    {
        m_Impl->fragment = fragment;
        m_Impl->hasFragment = true;
    }

    void Uri::clearFragment()
    {
        m_Impl->hasFragment = false;
    }

    void Uri::setHasPort(bool hasPort)
    {
        m_Impl->hasPort = hasPort;
    }

    void Uri::setPort(uint16_t port)
    {
        m_Impl->port = port;
    }

}

