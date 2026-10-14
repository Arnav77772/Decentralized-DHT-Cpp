
#include "Network.h"

#include <iostream>
#include <cstddef>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <unistd.h>
#endif

Network::Network()
    : serverSocket(-1),
      clientSocket(-1),
      connectedSocket(-1)
#ifdef _WIN32
      , winsockInitialized(false)
#endif
{
#ifdef _WIN32
    WSADATA data;

    if (WSAStartup(MAKEWORD(2, 2), &data) == 0)
        winsockInitialized = true;
    else
        std::cerr << "Winsock initialization failed.\n";
#endif
}

Network::~Network()
{
    closeConnection();

#ifdef _WIN32
    if (winsockInitialized)
        WSACleanup();
#endif
}

bool Network::startServer(int port)
{
#ifdef _WIN32
    if (!winsockInitialized)
        return false;
#endif

    if (port < 1 || port > 65535 || serverSocket >= 0)
    {
        std::cerr << "Invalid port or server already running.\n";
        return false;
    }

    serverSocket = static_cast<int>(
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

    if (serverSocket < 0)
    {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<unsigned short>(port));
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket,
             reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) < 0)
    {
        std::cerr << "Bind failed. Check whether the port is in use.\n";
        closeConnection();
        return false;
    }

    if (listen(serverSocket, 10) < 0)
    {
        std::cerr << "Listen failed.\n";
        closeConnection();
        return false;
    }

    std::cout << "Server listening on port " << port << '\n';
    return true;
}

bool Network::acceptClient()
{
    if (serverSocket < 0)
    {
        std::cerr << "Start the server first.\n";
        return false;
    }

    sockaddr_in address{};

#ifdef _WIN32
    int length = sizeof(address);
#else
    socklen_t length = sizeof(address);
#endif

    int accepted = static_cast<int>(
        accept(serverSocket,
               reinterpret_cast<sockaddr*>(&address),
               &length));

    if (accepted < 0)
    {
        std::cerr << "Accept failed.\n";
        return false;
    }

#ifdef _WIN32
    if (connectedSocket >= 0 && connectedSocket != clientSocket)
        closesocket(connectedSocket);
#else
    if (connectedSocket >= 0 && connectedSocket != clientSocket)
        close(connectedSocket);
#endif

    connectedSocket = accepted;
    std::cout << "Client connected.\n";
    return true;
}

bool Network::connectToServer(const std::string& ip, int port)
{
#ifdef _WIN32
    if (!winsockInitialized)
        return false;
#endif

    if (port < 1 || port > 65535 || clientSocket >= 0)
    {
        std::cerr << "Invalid port or client already connected.\n";
        return false;
    }

    clientSocket = static_cast<int>(
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

    if (clientSocket < 0)
    {
        std::cerr << "Client socket creation failed.\n";
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<unsigned short>(port));

#ifdef _WIN32
    unsigned long ipAddress = inet_addr(ip.c_str());

    if (ipAddress == INADDR_NONE && ip != "255.255.255.255")
#else
    if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) != 1)
#endif
    {
        std::cerr << "Invalid IPv4 address.\n";
        closeConnection();
        return false;
    }

#ifdef _WIN32
    address.sin_addr.s_addr = ipAddress;
#endif

    if (connect(clientSocket,
                reinterpret_cast<sockaddr*>(&address),
                sizeof(address)) < 0)
    {
        std::cerr << "Connection failed.\n";
        closeConnection();
        return false;
    }

    connectedSocket = clientSocket;
    std::cout << "Connected to " << ip << ':' << port << '\n';
    return true;
}

bool Network::sendMessage(const std::string& message)
{
    if (connectedSocket < 0 || message.empty())
        return false;

    const std::string packet = message + '\n';
    std::size_t sentTotal = 0;

    while (sentTotal < packet.size())
    {
#ifdef _WIN32
        int sent = send(
            connectedSocket,
            packet.c_str() + sentTotal,
            static_cast<int>(packet.size() - sentTotal),
            0);
#else
        ssize_t sent = send(
            connectedSocket,
            packet.c_str() + sentTotal,
            packet.size() - sentTotal,
            0);
#endif

        if (sent <= 0)
        {
            std::cerr << "Send failed.\n";
            return false;
        }

        sentTotal += static_cast<std::size_t>(sent);
    }

    return true;
}

std::string Network::receiveMessage()
{
    if (connectedSocket < 0)
        return "";

    std::string message;
    char buffer[512];

    while (message.size() <= 65536)
    {
#ifdef _WIN32
        int received = recv(
            connectedSocket, buffer,
            static_cast<int>(sizeof(buffer)), 0);
#else
        ssize_t received = recv(
            connectedSocket, buffer, sizeof(buffer), 0);
#endif

        if (received <= 0)
            return message;

        for (int i = 0; i < received; ++i)
        {
            if (buffer[i] == '\n')
                return message;

            message += buffer[i];

            if (message.size() > 65536)
            {
                std::cerr << "Message too large.\n";
                return "";
            }
        }
    }

    return "";
}

bool Network::isConnected() const
{
    return connectedSocket >= 0;
}

void Network::closeConnection()
{
#ifdef _WIN32
    if (connectedSocket >= 0)
        closesocket(connectedSocket);

    if (clientSocket >= 0 && clientSocket != connectedSocket)
        closesocket(clientSocket);

    if (serverSocket >= 0)
        closesocket(serverSocket);
#else
    if (connectedSocket >= 0)
        close(connectedSocket);

    if (clientSocket >= 0 && clientSocket != connectedSocket)
        close(clientSocket);

    if (serverSocket >= 0)
        close(serverSocket);
#endif

    serverSocket = -1;
    clientSocket = -1;
    connectedSocket = -1;
}
