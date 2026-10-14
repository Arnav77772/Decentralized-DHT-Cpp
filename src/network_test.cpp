
#include "Network.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    Network network;

    if (argc < 2)
    {
        std::cout << "Usage:\n";
        std::cout << "Server: network_test.exe server\n";
        std::cout << "Client: network_test.exe client\n";
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "server")
    {
        if (!network.startServer(5000))
            return 1;

        std::cout << "Waiting for client...\n";

        if (!network.acceptClient())
            return 1;

        std::string message = network.receiveMessage();
        std::cout << "Received: " << message << '\n';

        if (!message.empty())
            network.sendMessage("Hello from server!");

        network.closeConnection();
    }
    else if (mode == "client")
    {
        if (!network.connectToServer("127.0.0.1", 5000))
            return 1;

        network.sendMessage("Hello from client!");

        std::string reply = network.receiveMessage();
        std::cout << "Server replied: " << reply << '\n';

        network.closeConnection();
    }
    else
    {
        std::cout << "Unknown mode. Use server or client.\n";
        return 1;
    }

    return 0;
}
