#pragma once

#include <string>
#include <memory>

namespace http {

    /**
     * A HTTP request object that is responsible for parsing and storing the
     * data of HTTP requests. This implements the standard defined in (the RFC
     * standard)[https://tools.ietf.org/html/rfc2616].
     */
    class Request
    {
    public:
        Request();
        ~Request();

        /**
         * Parses from a stringified version of the HTTP request, storing the
         * data in the object's internal state.
         */
        bool parseFromString(const std::string& string);

        int httpVersionMajor() const;

        int httpVersionMinor() const;

        const std::string& method() const;

        const std::string& uri() const;

        bool hasHeader(const std::string& headerName) const;

        const std::string& header(const std::string& headerName) const;

        bool hasBody() const;

        const std::string& body() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

    class Response
    {
    public:
        Response();
        ~Response();

        std::string constructString() const;
        void clear();
        void setStatusCode(int code);
        void setReasonPhrase(const std::string& phrase);
        void setHeader(const std::string& headerName, const std::string& headerValue);
        void setBody(const std::string& body);
        void setVersionMajor(int versionMajor);
        void setVersionMinor(int versionMinor);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

}

