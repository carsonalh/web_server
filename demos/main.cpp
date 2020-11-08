#include <winsock2.h>
#include <ws2tcpip.h>

#include <web/http.hpp>
#include <web/uri.hpp>

#include <iostream>
#include <string>
#include <optional>
#include <sstream>

#include <cstring>
#include <cstdio>

#define DEFAULT_PORT "8080"
#define BACKLOG 24

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

class IServer
{
public:
    virtual ~IServer() = default;

    virtual std::string processRequest(const std::string& incoming) = 0;
};

class HttpServer : public IServer
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

int main() 
{
    WSADATA d;

    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        std::cerr << "failde to initialise winsock.\n";
        return 1;
    }

    SOCKET receiveSocket = NULL;

    struct addrinfo hints;
    struct addrinfo* bindAddress;

    std::memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(nullptr, DEFAULT_PORT, &hints, &bindAddress);

    receiveSocket = socket(
            bindAddress->ai_family,
            bindAddress->ai_socktype,
            bindAddress->ai_protocol);

    if (receiveSocket == INVALID_SOCKET) {
        std::cerr << "failed to create a valid socket.\n";
        return 1;
    }

    if (bind(receiveSocket, bindAddress->ai_addr, bindAddress->ai_addrlen)) {
        std::cerr << "faild to bind the created socket.\n";
        return 1;
    }

    if (listen(receiveSocket, BACKLOG) < 0) {
        std::cerr << "failed to get the socket listening.\n";
        return 1;
    }

    {
        IServer& server = HttpServer();
        struct sockaddr_storage clientAddress;
        socklen_t clientSize = sizeof clientAddress;
        while (true) {
            SOCKET clientSocket = accept(receiveSocket, (struct sockaddr*)&clientAddress, &clientSize);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "the socket from accept() was invalid.\n";
                WSACleanup();
                return 1;
            }
            std::ostringstream incomingMessage;
            char receiveBuffer[1024];
            int len = recv(clientSocket, receiveBuffer, sizeof receiveBuffer, 0);
            if (len > 0) {
                incomingMessage.write(receiveBuffer, len * sizeof receiveBuffer[0]);
            }
            std::string serverInput = incomingMessage.str();
            std::string serverOutput = server.processRequest(serverInput);
            int sent = send(clientSocket, serverOutput.c_str(), serverOutput.size(), 0);
            std::cout << "sent " << sent << " of " << serverOutput.size() << " bytes\n";
            closesocket(clientSocket);
        }
    }
}

