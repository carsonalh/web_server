#include "web/http.hpp"
#include "web/text.hpp"

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

        /**
         * Parses the http headers and stores them in this implementation
         * object.
         * @returns
         *      The first character immediately after the header section of the
         *      http request. It is up to the user of this function to make
         *      sure that the remaining characters are also valid; most
         *      specifically, that of the CRLF that should directly follow this
         *      part of the request.
         */
        int parseHttpHeaders(const std::string& string, int offset);

    public:
        std::string             method;
        std::string             uri;
        int                     httpVersionMajor = 0;
        int                     httpVersionMinor = 0;
        std::unordered_map<
            std::string,
            std::string>        headers;
        std::string             body;

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

        // make sure the request line ends with a CRLF

        {
            auto CRLF = "\r\n";
            if (string.size() <= (index - 1) + std::strlen(CRLF)) {
                return -1;
            }
            else if (std::memcmp(CRLF, string.data() + index, std::strlen(CRLF)) != 0) {
                return -1;
            }
            index += std::strlen(CRLF);
        }
        if (string.size() == index) {
            return -1;
        }

        return index;
    }

    int Request::Impl::parseHttpHeaders(const std::string& string, int offset)
    {
        auto index = offset;
        auto startIndex = index;

        std::string header, value;

        headers.clear();

        bool noValue = false;
        bool lastIterationWasReturn = false;

        while (index < string.size()) {
            if (string[index] == ':') {
                header = string.substr(startIndex, index - startIndex);
                for (auto c : header)
                    if (text::HTTP_HEADER_SEPARATORS.contains(c))
                        return -1;

                ++index;

                while (index < string.size() && string[index] == ' ') ++index;
                if (index < string.size() && string[index] == '\r') {
                    noValue = true;
                    startIndex = --index;
                }
                else {
                    startIndex = index;
                }
                lastIterationWasReturn = false;
            }
            else if (string[index] == '\r') {
                if (lastIterationWasReturn) {
                    break;
                }
                lastIterationWasReturn = true;
                auto endIndex = index;
                if (!noValue) {
                    while (std::isspace(string[endIndex - 1])) {
                        --endIndex;
                    }
                    value = string.substr(startIndex, endIndex - startIndex);
                }
                else {
                    value = "";
                }
                startIndex = ++index + 1;
                for (auto& c : header) c = std::tolower(c);
                headers.insert({ std::move(header), std::move(value) });
                noValue = false;
            }
            else {
                lastIterationWasReturn = false;
            }
            ++index;
        }

        return index;
    }

    bool Request::parseFromString(std::string string)
    {
        int end = 0;

        if ((end = m_Impl->parseHttpMethod(string, 0)) < 0)
            return false;

        ++end; // Advance past the trailing space

        if ((end = m_Impl->parseHttpUri(string, end)) < 0)
            return false;

        ++end; // Advance past the trailing space

        if ((end = m_Impl->parseHttpVersion(string, end)) < 0)
            return false;

        // parseHttpVersion would have already gone past the CRLF diretly after
        // it
        // "end" should now be at the first character of the headers

        if ((end = m_Impl->parseHttpHeaders(string, end)) < 0)
            return false;

        {
            auto CRLF = "\r\n";

            if (end + std::strlen(CRLF) < string.size()) {
                // TODO: make sure there is a CRLF inbetween the headers and the
                // body (and parse the body)
                end += std::strlen(CRLF); // advancing the CRLF sequence, but is not checking if it is correct
                m_Impl->body = string.substr(end);
            }
            else {
                m_Impl->body = "";
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
        std::string lowerCopy = headerName;
        for (auto& c : lowerCopy) c = std::tolower(c);
        return m_Impl->headers.find(lowerCopy) != m_Impl->headers.end();
    }

    std::string Request::header(const std::string& headerName) const
    {
        std::string lowerCopy = headerName;
        for (auto& c : lowerCopy) c = std::tolower(c);
        return m_Impl->headers.at(lowerCopy);
    }

    bool Request::hasBody() const
    {
        return !m_Impl->body.empty();
    }

    std::string Request::body() const
    {
        return m_Impl->body;
    }

}

