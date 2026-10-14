
#ifndef NETWORK_H
#define NETWORK_H

#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

class Network
{
public:
    Network();
    ~Network();

    // Start listening for incoming TCP connections.
    bool startServer(int port);

    // Accept one incoming client connection.
    bool acceptClient();

    // Connect to a remote server.
    bool connectToServer(const std::string& ip, int port);

    // Send and receive newline-terminated messages.
    bool sendMessage(const std::string& message);
    std::string receiveMessage();

    // Close sockets safely.
    void closeConnection();

    // Return true when a client/server connection exists.
    bool isConnected() const;

private:
    int serverSocket;
    int clientSocket;
    int connectedSocket;

#ifdef _WIN32
    bool winsockInitialized;
#endif
};

#endif
