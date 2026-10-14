
#include "DHT.h"

#include <iostream>

NodeID DHT::hashKey(const std::string& key)
{
    return hashString(key);
}

DHT::RingMap::const_iterator DHT::successorOf(int position) const
{
    if (ring.empty())
        return ring.end();

    auto it = ring.lower_bound(position);

    if (it == ring.end())
        return ring.begin();

    return it;
}

DHT::RingMap::const_iterator DHT::predecessorOf(int nodeID) const
{
    auto it = ring.find(nodeID);

    if (it == ring.end() || ring.empty())
        return ring.end();

    if (it == ring.begin())
        it = ring.end();

    --it;
    return it;
}

bool DHT::inRange(int h, int from, int to)
{
    if (from < to)
        return h > from && h <= to;

    if (from > to)
        return h > from || h <= to;

    return true;
}

bool DHT::addNode(int nodeID)
{
    if (nodeID < 0 || nodeID >= RING_SIZE)
    {
        std::cout << "Invalid node ID. Use 0-99.\n";
        return false;
    }

    if (ring.find(nodeID) != ring.end())
    {
        std::cout << "Node already exists.\n";
        return false;
    }

    if (ring.empty())
    {
        ring.emplace(
            nodeID,
            std::unique_ptr<Node>(new Node(nodeID))
        );

        std::cout << "Node " << nodeID << " joined the ring.\n";
        return true;
    }

    auto oldSuccessor = successorOf(nodeID);
    auto predecessor = predecessorOf(oldSuccessor->first);

    // Save the IDs before inserting the new node.
    int successorID = oldSuccessor->first;
    int predecessorID = predecessor->first;

    ring.emplace(
        nodeID,
        std::unique_ptr<Node>(new Node(nodeID))
    );

    // Move only keys that now belong to the new node.
    std::vector<std::pair<std::string, std::string>> oldEntries =
        ring.at(successorID)->entries();

    std::size_t moved = 0;

    for (const auto& entry : oldEntries)
    {
        int position = hashKey(entry.first);

        if (inRange(position, predecessorID, nodeID))
        {
            ring.at(successorID)->erase(entry.first);
            ring.at(nodeID)->store(entry.first, entry.second);
            ++moved;
        }
    }

    std::cout << "Node " << nodeID << " joined the ring. "
              << moved << " key(s) transferred.\n";

    return true;
}

bool DHT::removeNode(int nodeID)
{
    auto it = ring.find(nodeID);

    if (it == ring.end())
    {
        std::cout << "Node not found.\n";
        return false;
    }

    if (ring.size() == 1 && it->second->dataCount() > 0)
    {
        std::cout << "Cannot remove the last node while it holds data.\n";
        return false;
    }

    // Preserve the departing node's data before deleting it.
    std::vector<std::pair<std::string, std::string>> oldEntries =
        it->second->entries();

    ring.erase(it);

    // Redistribute its keys to their new responsible nodes.
    for (const auto& entry : oldEntries)
    {
        auto destination = successorOf(hashKey(entry.first));

        if (destination != ring.end())
            destination->second->store(entry.first, entry.second);
    }

    std::cout << "Node " << nodeID << " left the ring. "
              << oldEntries.size() << " key(s) redistributed.\n";

    return true;
}

void DHT::put(const std::string& key, const std::string& value)
{
    auto node = successorOf(hashKey(key));

    if (node == ring.end())
    {
        std::cout << "Cannot PUT: no nodes in the DHT.\n";
        return;
    }

    bool inserted = node->second->store(key, value);

    std::cout << (inserted ? "Key inserted" : "Key updated")
              << " on node " << node->first << ".\n";
}

void DHT::get(const std::string& key) const
{
    auto node = successorOf(hashKey(key));

    if (node == ring.end())
    {
        std::cout << "Cannot GET: no nodes in the DHT.\n";
        return;
    }

    std::string value;

    if (node->second->fetch(key, value))
    {
        std::cout << "Key: " << key
                  << ", Value: " << value
                  << ", Node: " << node->first << '\n';
    }
    else
    {
        std::cout << "Key not found: " << key << '\n';
    }
}

void DHT::remove(const std::string& key)
{
    auto node = successorOf(hashKey(key));

    if (node == ring.end())
    {
        std::cout << "Cannot DELETE: no nodes in the DHT.\n";
        return;
    }

    if (node->second->erase(key))
        std::cout << "Key deleted from node " << node->first << ".\n";
    else
        std::cout << "Key not found: " << key << '\n';
}

void DHT::showResponsibleNode(const std::string& key) const
{
    auto node = successorOf(hashKey(key));

    if (node == ring.end())
    {
        std::cout << "No nodes available in the DHT.\n";
        return;
    }

    std::cout << "Key: " << key
              << ", Hash position: " << hashKey(key)
              << ", Responsible node: " << node->first << '\n';
}

void DHT::displayNodes() const
{
    if (ring.empty())
    {
        std::cout << "No nodes in the DHT.\n";
        return;
    }

    std::cout << "Nodes in ring order:\n";

    for (const auto& entry : ring)
    {
        std::cout << "Node ID: " << entry.first
                  << ", Stored keys: " << entry.second->dataCount()
                  << '\n';
    }
}

void DHT::displayData() const
{
    if (ring.empty())
    {
        std::cout << "No nodes or data in the DHT.\n";
        return;
    }

    for (const auto& entry : ring)
    {
        entry.second->displayData();
    }
}

std::size_t DHT::nodeCount() const
{
    return ring.size();
}
