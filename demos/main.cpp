#include <winsock2.h>
#include <ws2tcpip.h>

#include <web/http.hpp>
#include <web/uri.hpp>

#include <iostream>
#include <sstream>
#include <cstring>

#define DEFAULT_PORT "8080"
#define BACKLOG 24

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

    struct sockaddr_storage clientAddress;
    socklen_t clientSize = sizeof clientAddress;
    int count = 0;
    while (true) {
        SOCKET clientSocket = accept(receiveSocket, (struct sockaddr*)&clientAddress, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "the socket from accept() was invalid.\n";
            return 1;
        }

        char receiveBuffer[1024];
        int len;
        len = recv(clientSocket, receiveBuffer, sizeof receiveBuffer, 0);
        std::string receiveString(receiveBuffer, len);
        http::Request request;
        request.parseFromString(receiveString);
        uri::Uri uri;
        uri.parseFromString(request.uri());
        if (uri.path().size() > 1 && uri.path()[1] == "favicon.ico") {
            std::cout << "asking for favicon.ico\n";
        }
        else {
            ++count;
        }

        std::ostringstream out;
        out << "<h1 style=\"color: blue;\">here is a web page " << count << "</h1>\r\n";
        http::Response res;
        res.setStatusCode(200);
        res.setReasonPhrase("OK");
        res.setHeader("Content-Type", "text/html");
        res.setBody(out.str());
        auto string = res.constructString();

        int sent = send(clientSocket, string.c_str(), string.size(), 0);

        std::cout << "sent " << sent << " of " << string.size() << " bytes\n";

        closesocket(clientSocket);
    }

    WSACleanup();
}
