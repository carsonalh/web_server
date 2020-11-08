#include "web/main.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <sstream>

#define DEFAULT_PORT "8080"
#define BACKLOG 24

namespace web {

    int runApplication(Application& app, int argc, const char** argv)
    {
        app.run();
        return 0;
    }

    int Application::run()
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
            IServer& server = this->server();
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

}

