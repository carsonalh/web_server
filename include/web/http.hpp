/**
 * @file web/http.hpp
 * Contains all the code for the HTTP implementation (both constructing, and
 * receiving messages).
 */
#pragma once

#include <string>
#include <memory>

namespace http {

    /**
     * @brief Represents an HTTP request.
     *
     * A HTTP request object that is responsible for parsing and storing the
     * data of HTTP requests. This implements the standard defined in [the RFC
     * standard](https://tools.ietf.org/html/rfc2616).
     */
    class Request
    {
    public:
        Request();
        ~Request();

        /**
         * @brief Parses from a stringified version of the HTTP request,
         * storing the data in the object's internal state.
         */
        bool parseFromString(const std::string& string);

        /** @brief Gets the major version of HTTP in the request. */
        int httpVersionMajor() const;

        /** @brief Gets the minor version of HTTP in the request. */
        int httpVersionMinor() const;

        /** @brief Gets the HTTP method of the request. */
        const std::string& method() const;

        /** @brief Gets the URI component of the request (aka. the path). */
        const std::string& uri() const;

        /** @brief Tells whether or not the request has a given header. */
        bool hasHeader(const std::string& headerName) const;

        /** @brief Gets a header from the request by name. */
        const std::string& header(const std::string& headerName) const;

        /**
         * @brief Tells whether or not the HTTP request has a body (as this is
         * optional).
         */
        bool hasBody() const;

        /** @brief Gets the body of the HTTP request. */
        const std::string& body() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

    /**
     * @brief Represents an HTTP response.
     *
     * A HTTP response object that is responsible for constructing HTTP
     * responses as strings. This implements the standard defined in [the RFC
     * standard](https://tools.ietf.org/html/rfc2616).
     */
    class Response
    {
    public:
        Response();
        ~Response();

        /** @brief Constructs the response string with the given data.  */
        std::string constructString() const;

        /** @brief Clears the internal state. */
        void clear();

        /** @brief Sets the status code of the HTTP response. */
        void setStatusCode(int code);

        /**
         * @brief Sets the "reason phrase" of the http response (terminology
         * defined in the RFC).
         *
         * For example, in `200 OK`, the reason phrase would be `OK`.
         *
         * As another example, in `500 Internal Server Error`, the reason
         * phrase would be `Internal Server Error`.
         */
        void setReasonPhrase(const std::string& reasonPhrase);

        /**
         * @brief Sets a header for the HTTP response.
         *
         * @param[in] headerName
         *      The name of the header.
         * @param[in] headerValue
         *      The value of the header. This will insert whatever string you
         *      put there, and will not do any sort of list operations for you;
         *      if you want to put a list in here, you have to join it with
         *      semicolons or commas manually.
         */
        void setHeader(const std::string& headerName, const std::string& headerValue);

        /** @brief Sets the body of the HTTP request.  */
        void setBody(const std::string& body);

        /**
         * @brief Sets the HTTP major version of the request.
         *
         * @note
         *      The default version for this class is 1.1, so for most cases,
         *      you won't have to change this.
         */
        void setVersionMajor(int versionMajor);

        /**
         * @brief Sets the HTTP minor version of the request.
         *
         * @note
         *      The default version for this class is 1.1, so for most cases,
         *      you won't have to change this.
         */
        void setVersionMinor(int versionMinor);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

}

