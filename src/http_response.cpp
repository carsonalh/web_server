#include "web/http.hpp"

#include <sstream>
#include <map>

namespace http {

    struct Response::Impl
    {
    public:
        void clear();

    public:
        std::string         reasonPhrase;
        int                 statusCode = 0;
        std::map<
            std::string,
            std::string>    headers;
        std::string         body;
        int                 versionMajor = 1;
        int                 versionMinor = 1;

    };

    void Response::Impl::clear()
    {
        reasonPhrase.clear();
        headers.clear();
        body.clear();
        statusCode = 0;
        versionMajor = 1;
        versionMinor = 1;
    }

    Response::Response()
        : m_Impl{ std::make_unique<Impl>() }
    {
    }

    Response::~Response()
    {
    }

    std::string Response::constructString() const
    {
        const auto CRLF = "\r\n";
        std::ostringstream out;
        out << "HTTP/" << m_Impl->versionMajor << '.' << m_Impl->versionMinor << ' ';
        out << m_Impl->statusCode << ' ';
        out << m_Impl->reasonPhrase << CRLF;
        for (const auto& pair : m_Impl->headers) {
            out << pair.first << ": " << pair.second << CRLF;
        }
        out << CRLF;
        out << m_Impl->body;
        return out.str();
    }

    void Response::clear()
    {
        m_Impl->clear();
    }

    void Response::setStatusCode(int code)
    {
        m_Impl->statusCode = code;
    }

    void Response::setReasonPhrase(const std::string& phrase)
    {
        m_Impl->reasonPhrase = phrase;
    }

    void Response::setHeader(const std::string& headerName, const std::string& headerValue)
    {
        m_Impl->headers.insert({ headerName, headerValue });
    }

    void Response::setBody(const std::string& body)
    {
        m_Impl->body = body;
    }

    void Response::setVersionMajor(int versionMajor)
    {
        m_Impl->versionMajor = versionMajor;
    }

    void Response::setVersionMinor(int versionMinor)
    {
        m_Impl->versionMinor = versionMinor;
    }

}
