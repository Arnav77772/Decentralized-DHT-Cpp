#ifndef DHT_H
#define DHT_H

#include <cstddef>
#include <map>
#include <memory>
#include <string>

#include "Hashing.h"
#include "Node.h"

class DHT
{
public:
    using RingMap = std::map<NodeID, std::unique_ptr<Node>>;

    DHT() = default;

    static NodeID hashKey(const std::string& key);

    bool addNode(NodeID nodeID);
    bool removeNode(NodeID nodeID);

    void put(const std::string& key, const std::string& value);
    void get(const std::string& key) const;
    void remove(const std::string& key);
    void showResponsibleNode(const std::string& key) const;
    void displayNodes() const;
    void displayData() const;

    std::size_t nodeCount() const;

private:
    RingMap ring;

    RingMap::const_iterator successorOf(NodeID position) const;
    RingMap::const_iterator predecessorOf(NodeID nodeID) const;

    static bool inRange(NodeID h, NodeID from, NodeID to);
};

#endif
