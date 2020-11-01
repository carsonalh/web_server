#include "uri/uri.hpp"

#include <string>
#include <vector>
#include <regex>
#include <memory>

namespace Uri {

    struct Uri::Impl
    {
        std::string                 scheme;
        std::string                 host;
        std::vector<std::string>    path;
        std::string                 query;
        std::string                 fragment;
        bool                        has_port;
        uint16_t                    port;

    public:
        void Clear()
        {
            scheme.clear();
            host.clear();
            path.clear();
            query.clear();
            fragment.clear();
            has_port = false;
            port = 0;
        }
    };

    Uri::Uri()
        : m_Impl{ std::make_unique<Uri::Impl>() }
    {
    }

    Uri::~Uri()
    {
    }

    bool Uri::ParseFromString(std::string string)
    {
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

        std::regex scheme_pattern{ "^([a-zA-Z][a-zA-Z0-9+\\-.]*):" };
        m_Impl->scheme = search(scheme_pattern, 1);

        std::regex host_pattern{ "^//(([a-zA-Z0-9\\-._~]|%[a-fA-F0-9]{2}|[!$&'()*+,;=])+)" };
        m_Impl->host = search(host_pattern, 1);

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

    const std::string& Uri::GetScheme() const
    {
        return m_Impl->scheme;
    }

    const std::string& Uri::GetHost() const
    {
        return m_Impl->host;
    }

    const std::vector<std::string>& Uri::GetPath() const
    {
        return m_Impl->path;
    }

    const std::string& Uri::GetQuery() const
    {
        return m_Impl->query;
    }

    const std::string& Uri::GetFragment() const
    {
        return m_Impl->fragment;
    }

    bool Uri::HasPort() const
    {
        return m_Impl->has_port;
    }

    uint16_t Uri::GetPort() const
    {
        return m_Impl->port;
    }

}
