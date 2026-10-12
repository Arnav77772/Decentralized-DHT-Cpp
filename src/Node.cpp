
#include "Node.h"
#include <iostream>

Node::Node(int id) : nodeID(id) {}

int Node::getID() const
{
    return nodeID;
}

bool Node::store(const std::string& key, const std::string& value)
{
    return localTable.put(key, value);
}

bool Node::fetch(const std::string& key, std::string& valueOut) const
{
    return localTable.get(key, valueOut);
}

bool Node::erase(const std::string& key)
{
    return localTable.remove(key);
}

std::size_t Node::dataCount() const
{
    return localTable.size();
}

std::vector<std::pair<std::string, std::string>> Node::entries() const
{
    return localTable.entries();
}

void Node::displayData() const
{
    std::cout << "Node " << nodeID << " data:\n";
    localTable.display();
}