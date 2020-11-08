#include <web/http.hpp>
#include <web/uri.hpp>
#include <web/main.hpp>

#include <string>
#include <optional>
#include <sstream>
#include <cstdio>

namespace utils {
    std::optional<std::string> readFile(const std::string& fileName)
    {
        FILE* f = fopen(fileName.c_str(), "rb");
        std::optional<std::string> result;
        if (f == NULL) {
            return result; // empty
        }
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* buffer = new char[size + 1];
        fread(buffer, sizeof *buffer, size, f);
        result = std::string{ buffer, size };
        fclose(f);
        return result;
    }
}

class HttpServer : public web::IServer
{
public:
    std::string processRequest(const std::string& incoming) override
    {
        http::Request request;
        std::string path;
        {
            uri::Uri uri;
            request.parseFromString(incoming);
            uri.parseFromString(request.uri());
            std::ostringstream out;
            if (uri.path().size() < 2 && uri.path()[0] == "") {
                out << '/';
            }
            for (int i = 1; i < uri.path().size(); ++i) {
                out << '/' << uri.path()[i];
            }
            path = out.str();
        }
        http::Response response;
        {
            std::string fileLocation = "." + path;
            auto fileContents = utils::readFile(fileLocation);
            auto message = fileContents ? *fileContents : "file not found\r\n";
            message += path;
            std::string extension = (fileLocation.find(".html") == std::string::npos) ? "plain" : "html";
            if (extension == "plain") {
                extension = (fileLocation.find(".css") == std::string::npos) ? "plain" : "css";
            }
            response.setStatusCode(200);
            response.setReasonPhrase("OK");
            response.setHeader("Content-Type", "text/" + extension);
            response.setHeader("Content-Length", std::to_string(message.size()));
            response.setBody(message);
        }
        return response.constructString();
    }

protected:
    int m_Count = 0;

};

class HttpApplication : public web::Application
{
public:
    HttpApplication()
        : m_Server{}
    {
    }

    ~HttpApplication() = default;

    web::IServer& server() override
    {
        return m_Server;
    }

protected:
    HttpServer m_Server;
};

int main(int argc, const char** argv)
{
    HttpApplication app;
    return web::runApplication(app, argc, argv);
}

