
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

    bool startServer(int port);

    bool acceptClient();

    bool connectToServer(const std::string& ip, int port);

    bool sendMessage(const std::string& message);
    std::string receiveMessage();

    void closeConnection();

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
