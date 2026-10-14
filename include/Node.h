#ifndef NODE_H
#define NODE_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "Hashing.h"
#include "HashTable.h"

class Node
{
public:
    explicit Node(NodeID id);

    NodeID getID() const;

    bool store(const std::string& key, const std::string& value);
    bool fetch(const std::string& key, std::string& valueOut) const;
    bool erase(const std::string& key);

    std::size_t dataCount() const;
    std::vector<std::pair<std::string, std::string>> entries() const;

    void displayData() const;

private:
    NodeID nodeID;
    HashTable localTable;
};

#endif
