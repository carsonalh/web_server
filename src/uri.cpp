#include "uri/uri.hpp"

#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <memory>

namespace Uri {

    /** Implementation details of the Uri::Uri class. */
    struct Uri::Impl
    {
        std::string                 scheme;
        std::string                 user_info;
        std::string                 host;
        std::vector<std::string>    path;
        std::string                 query;
        std::string                 fragment;
        bool                        has_port;
        uint16_t                    port;

    public:
        /** Clears all the member data of the implementation.  */
        void Clear()
        {
            scheme.clear();
            user_info.clear();
            host.clear();
            path.clear();
            query.clear();
            fragment.clear();
            has_port = false;
            port = 0;
        }
    };

    bool Uri::isIpv4String(const std::string& string)
    {
        if (string.empty())
            return false;

        bool initialized = false;
        uint16_t currentNumber = 0;
        for (int i = 0; i < string.size(); ++i) {
            if (string[i] == '.') {
                if (!initialized || (currentNumber & ~0x00ff)) {
                    return false;
                }
                else {
                    initialized = false;
                    currentNumber = 0;
                    continue;
                }
            }
            else if ('0' <= string[i] && string[i] <= '9') {
                initialized = true;
                currentNumber *= 10;
                currentNumber += string[i] - '0';
            }
            else {
                return false;
            }
        }

        return (
            initialized
            && !(currentNumber & ~0x00ff)
            );
    }

    bool Uri::isIpv6String(const std::string& string)
    {
        if (string.empty())
            return false;

        int numSegments;

        {
            numSegments = 1;

            int numSingleColons = 0;
            int numDoubleColons = 0;

            int currentColonRun = 0, currentHexRun = 0;
            for (int i = 0; i < string.size(); ++i) {
                if (string[i] == ':') {
                    currentHexRun = 0;
                    if (currentColonRun >= 2) {
                        return false;
                    }
                    else if (currentColonRun == 1) {
                        if (numDoubleColons > 0) {
                            return false;
                        }

                        ++currentColonRun;
                        ++numDoubleColons;
                        ++numSegments;
                    }
                    else {
                        ++currentColonRun;
                    }
                }
                else {
                    auto c = string[i];
                    if (
                        ('0' <= c && c <= '9')
                        || ('a' <= c && c <= 'f')
                        || ('A' <= c && c <= 'F')
                        ) {
                        if (currentColonRun > 0) {
                            ++numSingleColons;
                            ++numSegments;
                            currentColonRun = 0;
                        }

                        const size_t maxNumHexAllowedIpv6 = 4;
                        ++currentHexRun;
                        if (currentHexRun > maxNumHexAllowedIpv6) {
                            return false;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < string.size(); ++i) {
            char c = string[i];
            if (
                !('0' <= c && c <= '9')
                && !('a' <= c && c <= 'f')
                && !('A' <= c && c <= 'F')
                && !(c == ':')
                )
                return false;
        }

        // The greatest allowed amount of segments in an ipv6 address
        const size_t maxSegments = 8;

        if (numSegments > maxSegments)
            return false;

        return true;
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

        std::smatch search_results;

        m_Impl->Clear();

        auto search = [&string, &search_results](const std::regex& pattern, int result_index)
                -> std::string {
            auto found = std::regex_search(string, search_results, pattern);

            if (found) {
                auto& result = search_results[result_index].str();
                string = search_results.suffix().str();
                return std::move(result);
            }
            else {
                return "";
            }
        };

        if (std::regex_search(string, search_results, std::regex("^([a-zA-Z][a-zA-Z0-9+\\-.]*):"))) {
            m_Impl->scheme = std::move(search_results[1].str());
            string = std::move(search_results.suffix().str());
        }
        else if (std::regex_search(string, std::regex("^.*://"))) {
            return false;
        }

        {
            std::regex user_info_and_host_pattern{
                "^:?//((([a-zA-Z0-9\\-._~:!$&'\\(\\)*+,;=]|%[0-9a-fA-F]{2})*)@)?"
                "(([a-zA-Z0-9\\-._~]|%[a-fA-F0-9]{2}|[!$&'()*+,;=])+)"
            };

            if (std::regex_search(string, search_results, user_info_and_host_pattern)) {
                m_Impl->user_info = std::move(search_results[2].str());
                m_Impl->host = std::move(search_results[4].str());
                string = std::move(search_results.suffix().str());
            }
        }

        std::regex port_pattern{ "^:([0-9]{0,5})" };

        {
            const auto& port = search(port_pattern, 1);
            m_Impl->has_port = !port.empty();

            uint32_t port_number = 0;
            const uint32_t port_mask = 0x0000ffff;

            for (auto c : port) {
                // The regex has already guaranteed that c will be numeric
                port_number *= 10;
                port_number += c - '0';

                if (port_number & ~port_mask) {
                    m_Impl->has_port = false;
                    return false;
                }
            }

            m_Impl->port = port_number;
        }

        std::regex path_pattern{
            "^("
                "/?(([a-zA-Z0-9\\-._~!$&'()*+,;=]|%[a-fA-F0-9]{2})*/)*"
                "([a-zA-Z0-9\\-._~!$&'()*+,;=]|%[a-fA-F0-9]{2})*"
            ")"
        };
        
        {
            const auto& result = search(path_pattern, 1);

            if (result.size() > 0) {
                for (int i = 0; i < result.size(); ++i) {
                    auto end = i;

                    while (end < result.size() && result[end] != '/') {
                        ++end;
                    }

                    m_Impl->path.push_back(std::move(result.substr(i, end - i)));

                    i = end;
                }
            }
        }

        std::regex query_pattern{ "^\\?(([a-zA-Z0-9\\-._~!$&'()*+,;=/?:@]|%[a-fA-F0-9]{2})*)" };
        m_Impl->query = search(query_pattern, 1);

        std::regex fragment_pattern{ "^#(([a-zA-Z0-9\\-._~!$&'()*+,;=/?:@]|%[a-fA-F0-9]{2})*)" };
        m_Impl->fragment = search(fragment_pattern, 1);

        if (m_Impl->fragment.size() > 0) {
            if (search_results.suffix().str().size() > 0) {
                m_Impl->Clear();
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
        return m_Impl->user_info;
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
        return m_Impl->has_port;
    }

    uint16_t Uri::port() const
    {
        return m_Impl->port;
    }

    bool Uri::containsRelativePath() const
    {
        return m_Impl->path.size() && !m_Impl->path[0].empty();
    }

    // TODO: this method should be refactored
    std::string Uri::constructString() const
    {
        std::ostringstream path{ "" };
        for (const auto& segment : m_Impl->path) {
            if (!segment.empty())
                path << "/" << segment;
        }

        std::string scheme_part, port_part, query_part, fragment_part;

        if (m_Impl->scheme.size() > 0) {
            scheme_part = m_Impl->scheme + ':';
        }

        if (m_Impl->has_port) {
            port_part = ':' + std::to_string(m_Impl->port);
        }

        if (m_Impl->query.size() > 0) {
            query_part = '?' + m_Impl->query;
        }

        if (m_Impl->fragment.size() > 0) {
            fragment_part = '#' + m_Impl->fragment;
        }

        return scheme_part + "//" + m_Impl->host + port_part + path.str() + query_part + fragment_part;
    }

    void Uri::setScheme(const std::string& scheme)
    {
        m_Impl->scheme = scheme;
    }

    void Uri::setUserInfo(const std::string& userInfo)
    {
        m_Impl->user_info = userInfo;
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
    }

    void Uri::setFragment(const std::string& fragment)
    {
        m_Impl->fragment = fragment;
    }

    void Uri::setHasPort(bool hasPort)
    {
        m_Impl->has_port = hasPort;
    }

    void Uri::setPort(uint16_t port)
    {
        m_Impl->port = port;
    }

}
