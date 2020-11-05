#include "http/http.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <cctype>

namespace http {

    /** Implementation of the http::Request class. */
    struct Request::Impl
    {
    public:
        /**
         * Parses the http method from the request, storing the method's value
         * in the impl struct.
         * @returns
         *      The index of the character immediately after the method part of
         *      the the http request string. If this method fails, it will
         *      return -1.
         */
        int parseHttpMethod(const std::string& string, int offset);

        /**
         * Parses the uri part of the http request from the request line.
         * @returns
         *      The index of the character immediately after the uri part of
         *      the http request string. If this method fails, it will return
         *      -1.
         */
        int parseHttpUri(const std::string& string, int offset);

        /**
         * Parses the version part of the http request from the request line.
         * @returns
         *      The index of the character immediately after the uri part of
         *      the http request string, which in this case should be a return
         *      character (\r), but that is outside of the scope of this
         *      function. If this method fails, it will return -1.
         */
        int parseHttpVersion(const std::string& string, int offset);

    public:
        std::string             method;
        std::string             uri;
        int                     httpVersionMajor = 0;
        int                     httpVersionMinor = 0;
        std::unordered_map<
            std::string,
            std::string>        headers;

    };

    Request::Request()
        : m_Impl{ std::make_unique<Impl>() }
    {
    }

    Request::~Request() = default;

    int Request::Impl::parseHttpMethod(const std::string& string, int offset)
    {
        int index = offset;

        while (index < string.size() && std::isalnum(string[index])) {
            ++index;
        }

        if (index >= string.size() || index == 0) {
            return -1;
        }

        if (string[index] != ' ') {
            return -1;
        }

        method = string.substr(0, index);

        return index;
    }

    int Request::Impl::parseHttpUri(const std::string& string, int offset)
    {
        int index = offset;

        while (index <= string.size() && string[index] != ' ') {
            ++index;
        }

        if (index - offset == 0)
            return -1;

        uri = string.substr(offset, index - offset);

        return index;
    }

    int Request::Impl::parseHttpVersion(const std::string& string, int offset)
    {
        const static char* httpString = "HTTP/";
        if (std::memcmp(httpString, string.data() + offset, std::strlen(httpString)) != 0) {
            return -1;
        }

        auto index = offset + std::strlen(httpString) - 1;

        // now verify and parse the actual version

        ++index;

        if (!std::isdigit(string[index])) {
            return -1;
        }

        // TODO: eventually refactor this into a function of its own
        int versionMajor = 0;
        while (std::isdigit(string[index])) {
            versionMajor *= 10;
            versionMajor += string[index] - '0';
            ++index;
        }

        if (string[index] != '.') {
            return -1;
        }
        else {
            ++index;
        }

        int versionMinor = 0;
        while (index < string.size() && std::isdigit(string[index])) {
            versionMinor *= 10;
            versionMinor += string[index] - '0';
            ++index;
        }

        httpVersionMajor = versionMajor;
        httpVersionMinor = versionMinor;

        return index;
    }

    bool Request::parseFromString(std::string string)
    {
        size_t end = 0;

        if ((end = m_Impl->parseHttpMethod(string, 0)) < 0)
            return false;

        if ((end = m_Impl->parseHttpUri(string, ++end)) < 0)
            return false;

        if ((end = m_Impl->parseHttpVersion(string, ++end)) < 0)
            return false;

        // make sure the request line ends with a CRLF

        {
            auto CRLF = "\r\n";
            if (string.size() <= (end - 1) + std::strlen(CRLF)) {
                return false;
            }
            else if (std::memcmp(CRLF, string.data() + end, std::strlen(CRLF)) != 0) {
                return false;
            }
            end += std::strlen(CRLF);
        }

        if (string.size() == end) {
            return false;
        }
        
        auto index = end;

        size_t startIndex = index;

        std::string header, value;

        m_Impl->headers.clear();

        while (index < string.size()) {
            if (string[index] == ':') {
                if (index + 1 >= string.size()) {
                    return false;
                }
                else if (string[index + 1] != ' ') {
                    return false;
                }
                header = string.substr(startIndex, index - startIndex);
                startIndex = ++index + 1;
            }
            else if (string[index] == '\r') {
                value = string.substr(startIndex, index - startIndex);
                startIndex = ++index + 1;
                m_Impl->headers.insert({ std::move(header), std::move(value) });
            }
            else {
                ++index;
            }
        }

        return true;
    }

    std::string Request::method() const
    {
        return m_Impl->method;
    }

    std::string Request::uri() const
    {
        return m_Impl->uri;
    }

    int Request::httpVersionMajor() const
    {
        return m_Impl->httpVersionMajor;
    }

    int Request::httpVersionMinor() const
    {
        return m_Impl->httpVersionMinor;
    }

    bool Request::hasHeader(const std::string& headerName) const
    {
        return m_Impl->headers.find(headerName) != m_Impl->headers.end();
    }

    std::string Request::header(const std::string& headerName) const
    {
        return m_Impl->headers.at(headerName);
    }

}

