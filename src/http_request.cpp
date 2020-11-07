#include "web/http.hpp"
#include "web/text.hpp"

#include <string>
#include <cstring>
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

        /**
         * Parses the body of a HTTP request, taking in the offset at which the
         * body should start relative to the given string of the whole request.
         * @returns
         *      The character at the end of the body if successful (which
         *      should be equal to string.size()), or a value less than zero if
         *      unsuccessful.
         */
        int parseHttpBody(const std::string& string, int offset);

        /**
         * Clears all the data from the implementation struct.
         */
        void clear();

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

        // if the first character of the header block is a CR, then there are
        // no headers!
        if (string[index] == '\r') {
            return index;
        }

        while (index < string.size()) {
            if (string[index] == '\r') {
                if (startIndex == index) {
                    // we're at the end of the header sequence
                    // double check the next two chars are CRLF sequence, if not, that's an error
                    auto CRLF = "\r\n";
                    if (index + std::strlen(CRLF) > string.size()) {
                        return -1;
                    }
                    else if (std::strncmp(string.data() + index, CRLF, std::strlen(CRLF)) != 0) {
                        return -1;
                    }
                    // if they are, all good to return true
                    return index;
                }

                // parse this current header
                int colonIndex = -1;
                for (int i = index - 1; i >= startIndex; --i) {
                    if (string[i] == ':') {
                        colonIndex = i;
                        break;
                    }
                }
                if (colonIndex == -1) {
                    return -1;
                }
                // validate the header
                header = string.substr(startIndex, colonIndex - startIndex);
                for (auto& c : header) {
                    if (text::HTTP_HEADER_SEPARATORS.contains(c)) {
                        return -1;
                    }
                    c = std::tolower(c);
                }

                // parse the current value
                int valueStart = colonIndex + 1;
                int valueEnd = index;
                while (std::isspace(string[valueStart])) ++valueStart;
                while (valueEnd > valueStart && std::isspace(string[valueEnd - 1])) --valueEnd;
                value = string.substr(valueStart, valueEnd - valueStart);

                // header and value aren't read from -- only written to which
                // won't occur until the next iteration; it is safe to move
                // them
                headers.insert({ std::move(header), std::move(value) });

                // check if it is a double CRLF sequence (if it is, end)
                //      make sure it is at least a single CRLF sequence
                auto CRLF = "\r\n";
                if (index + 1 >= string.size()) {
                    return -1;
                }
                else if (std::strncmp(string.data() + index, CRLF, std::strlen(CRLF)) != 0) {
                    return -1;
                }

                // move past the CRLF sequence for the next header (or the end)
                startIndex = ++index + 1;
            }
            ++index;
        }

        return index;
    }

    int Request::Impl::parseHttpBody(const std::string& string, int offset)
    {
        auto CRLF = "\r\n";
        auto index = offset;

        if (index >= string.size()) {
            return -1;
        }

        if (index + std::strlen(CRLF) < string.size()) {
            // Advancing the CRLF sequence, but is not checking if it is
            // correct, because we know that is handled when the headers are
            // parsed; see ::http::Request::Impl::parseHttpHeaders.
            index += std::strlen(CRLF);

            body = string.substr(index);
        }
        else {
            body = "";
        }

        index += body.size();

        return index;
    }

    void Request::Impl::clear()
    {
        method.clear();
        uri.clear();
        headers.clear();
        body.clear();
        httpVersionMajor = 0;
        httpVersionMinor = 0;
    }

    bool Request::parseFromString(const std::string& string)
    {
        m_Impl->clear();

        auto done = [this]() { m_Impl->clear(); return false; };

        int end = 0;

        if ((end = m_Impl->parseHttpMethod(string, 0)) < 0)
            return done();

        ++end; // Advance past the trailing space

        if ((end = m_Impl->parseHttpUri(string, end)) < 0)
            return done();

        ++end; // Advance past the trailing space

        if ((end = m_Impl->parseHttpVersion(string, end)) < 0)
            return done();

        // parseHttpVersion would have already gone past the CRLF diretly after
        // it
        // "end" should now be at the first character of the headers

        if ((end = m_Impl->parseHttpHeaders(string, end)) < 0)
            return done();

        // At this point, end should be on the CRLF before the body, but
        // parseHttpHeaders has already verified that it is indeed a CRLF,
        // because it needs the double CRLF sequence to terminate.

        if ((end = m_Impl->parseHttpBody(string, end)) < 0)
            return done();

        return true;
    }

    const std::string& Request::method() const
    {
        return m_Impl->method;
    }

    const std::string& Request::uri() const
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

    const std::string& Request::header(const std::string& headerName) const
    {
        std::string lowerCopy = headerName;
        for (auto& c : lowerCopy) c = std::tolower(c);
        return m_Impl->headers.at(lowerCopy);
    }

    bool Request::hasBody() const
    {
        return !m_Impl->body.empty();
    }

    const std::string& Request::body() const
    {
        return m_Impl->body;
    }

}

