#include <iostream>
#include <limits>
#include <string>
#include <cstdint>
#include "Hashing.h"

#include "DHT.h"

using namespace std;

NodeID readNodeID(const std::string& prompt)
{
    std::string input;

    std::cout << prompt;
    std::cin >> input;

    try
    {
        return static_cast<NodeID>(std::stoull(input));
    }
    catch (...)
    {
        std::cout << "Invalid node ID.\n";
        return 0;
    }
}

int readInt(const string& prompt)
{
    int value = 0;

    cout<<prompt;

    while (!(cin >> value))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Please enter a number: ";
    }

    return value;
}

string readWord(const string& prompt)
{
    string word;

    cout << prompt;
    cin >> word;

    return word;
}


void printMenu()
{
    cout << "\n=====================================\n";
    cout << "        DISTRIBUTED HASH TABLE\n";
    cout << "=====================================\n";
    cout << "1. Add node (JOIN)\n";
    cout << "2. Remove node (LEAVE)\n";
    cout << "3. Display nodes (ring order)\n";
    cout << "4. PUT\n";
    cout << "5. GET\n";
    cout << "6. DELETE\n";
    cout << "7. Display DHT data\n";
    cout << "8. Find responsible node for a key\n";
    cout << "9. Exit\n";
}




int main()
{
    DHT dht;

    string ip;
    unsigned int port;

    cout << "Enter this computer's IP address: ";
    cin >> ip;

    cout << "Enter port (1024-65535): ";
    if (!(cin >> port) || port < 1024 || port > 65535)
    {
        cout << "Invalid port number.\n";
        return 1;
    }

    NodeID nodeID = hashNode(
        ip, static_cast<unsigned short>(port)
    );

    cout << "Generated 64-bit Node ID: " << nodeID << '\n';

    while (true)
    {
        printMenu();

        int choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
        {
            if (dht.addNode(nodeID))
                cout << "Node joined successfully.\n";
            else
                cout << "Node already exists or could not join.\n";

            break;
        }

        case 2:
        {
            NodeID id = readNodeID("Enter Node ID to remove: ");
            dht.removeNode(id);
            break;
        }

        case 3:
            dht.displayNodes();
            break;

        case 4:
        {
            string key = readWord("Enter key: ");
            string value = readWord("Enter value: ");
            dht.put(key, value);
            break;
        }

        case 5:
            dht.get(readWord("Enter key: "));
            break;

        case 6:
            dht.remove(readWord("Enter key: "));
            break;

        case 7:
            dht.displayData();
            break;

        case 8:
            dht.showResponsibleNode(readWord("Enter key: "));
            break;

        case 9:
            cout << "Program ended.\n";
            return 0;

        default:
            cout << "Invalid choice.\n";
        }
    }
}
