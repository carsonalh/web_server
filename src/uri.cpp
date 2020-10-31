#include "uri/uri.hpp"

#include <string>
#include <numeric>
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

    public:
        void Clear()
        {
            scheme.clear();
            host.clear();
            path.clear();
            query.clear();
            fragment.clear();
        }
    };

    Uri::Uri()
        : m_Impl(new Impl)
    {
    }

    Uri::~Uri()
    {
        delete m_Impl;
    }

    bool Uri::ParseFromString(std::string string)
    {
        std::smatch search_results;

        m_Impl->Clear();

        std::regex scheme_pattern{ "^([a-zA-Z][a-zA-Z0-9+\\-.]*):" };

        auto found_scheme = std::regex_search(string, search_results, scheme_pattern);

        if (found_scheme) {
            auto result = search_results[1];
            m_Impl->scheme = result.str();
            string = std::move(search_results.suffix().str());
        }

        std::regex host_pattern{ "^(//)?(([a-zA-Z0-9\\-._~]|%[a-fA-F0-9]{2}|[!$&'()*+,;=])+)" };

        auto found_host = std::regex_search(string, search_results, host_pattern);

        if (found_host) {
            auto result = search_results[2];
            m_Impl->host = result.str();
            string = std::move(search_results.suffix().str());
        }

        std::regex path_pattern{
            "^("
                "/?(([a-zA-Z0-9\\-._~!$&'()*+,;=]|%[a-fA-F0-9]{2})*/)*"
                "([a-zA-Z0-9\\-._~!$&'()*+,;=]|%[a-fA-F0-9]{2})*"
            ")"
        };

        auto found_path = std::regex_search(string, search_results, path_pattern);

        if (found_path) {
            auto& result = search_results[1].str();

            for (int i = 0; i < result.size(); ++i) {
                auto end = i;

                while (end < result.size() && result[end] != '/') {
                    ++end;
                }

                m_Impl->path.push_back(std::move(result.substr(i, end - i)));

                i = end;
            }

            string = std::move(search_results.suffix().str());
        }

        std::regex query_pattern{ "^\\?(([a-zA-Z0-9\\-._~!$&'()*+,;=/?:@]|%[a-fA-F0-9]{2})*)" };

        auto found_query = std::regex_search(string, search_results, query_pattern);

        if (found_query) {
            auto& result = search_results[1].str();
            m_Impl->query = result;
            string = std::move(search_results.suffix().str());
        }

        std::regex fragment_pattern{ "^#(([a-zA-Z0-9\\-._~!$&'()*+,;=/?:@]|%[a-fA-F0-9]{2})*)" };

        auto found_fragment = std::regex_search(string, search_results, fragment_pattern);

        if (found_fragment) {
            auto& result = search_results[1].str();
            m_Impl->fragment = result;

            if (search_results.suffix().str().size() != 0) {
                m_Impl->Clear();
                return false;
            }
        }

        return true;
    }

    std::string Uri::GetScheme() const
    {
        return m_Impl->scheme;
    }

    std::string Uri::GetHost() const
    {
        return m_Impl->host;
    }

    const std::vector<std::string>& Uri::GetPath() const
    {
        return m_Impl->path;
    }

    std::string Uri::GetQuery() const
    {
        return m_Impl->query;
    }

    std::string Uri::GetFragment() const
    {
        return m_Impl->fragment;
    }

}
