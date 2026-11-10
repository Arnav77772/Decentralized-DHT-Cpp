
#include "DHT.h"
#include "Node.h"

#include <iostream>
#include <iterator>
#include <utility>

using namespace std;

NodeID DHT::hashKey(const string& key)
{
    return hashString(key);
}

DHT::RingMap::const_iterator DHT::successorOf(NodeID position) const
{
    if (ring.empty())
        return ring.end();

    auto it = ring.lower_bound(position);

    if (it == ring.end())
        return ring.begin();

    return it;
}

DHT::RingMap::const_iterator DHT::predecessorOf(NodeID nodeID) const
{
    if (ring.empty())
        return ring.end();

    auto it = ring.lower_bound(nodeID);

    if (it == ring.begin() || it == ring.end())
        return prev(ring.end());

    return prev(it);
}

bool DHT::inRange(NodeID h, NodeID from, NodeID to)
{
    if (from < to)
        return h > from && h <= to;

    if (from > to)
        return h > from || h <= to;

    return true;
}

bool DHT::addNode(NodeID nodeID)
{
    lock_guard<mutex> lock(ringMutex);

    if (ring.find(nodeID) != ring.end())
    {
        cout << "Node already exists.\n";
        return false;
    }

    if (ring.empty())
    {
        ring.emplace(nodeID, make_unique<Node>(nodeID));
        rebuildReplicas();

        cout << "Node " << nodeID << " joined the DHT.\n";
        return true;
    }

    auto successor = successorOf(nodeID);

    if (successor == ring.end())
        return false;

    auto predecessor = predecessorOf(successor->first);

    NodeID successorID = successor->first;
    NodeID predecessorID = predecessor->first;

    auto newNode = make_unique<Node>(nodeID);

    vector<pair<string, string>> oldEntries =
        successor->second->entries();

    for (const auto& entry : oldEntries)
    {
        NodeID keyID = hashKey(entry.first);

        if (inRange(keyID, predecessorID, nodeID))
        {
            newNode->store(entry.first, entry.second);
            successor->second->erase(entry.first);
        }
    }

    ring.emplace(nodeID, move(newNode));

    rebuildReplicas();

    cout << "Node " << nodeID << " joined the DHT.\n";
    return true;
}

bool DHT::removeNode(NodeID nodeID)
{
    lock_guard<mutex> lock(ringMutex);

    auto nodeIt = ring.find(nodeID);

    if (nodeIt == ring.end())
    {
        cout << "Node not found.\n";
        return false;
    }

    if (ring.size() == 1)
    {
        if (nodeIt->second->dataCount() > 0)
        {
            cout << "Cannot remove the last node because it contains data.\n";
            return false;
        }

        ring.erase(nodeIt);
        replicas.clear();

        cout << "Node " << nodeID << " left the DHT.\n";
        return true;
    }

    auto successor = next(nodeIt);

    if (successor == ring.end())
        successor = ring.begin();


        vector<pair<string, string>> entries =
        nodeIt->second->entries();

    for (const auto& entry : entries)
    {
        successor->second->store(entry.first, entry.second);
    }

    ring.erase(nodeIt);

    rebuildReplicas();

    cout << "Node " << nodeID << " left the DHT.\n";
    return true;
}

void DHT::put(const string& key, const string& value)
{
    lock_guard<mutex> lock(ringMutex);

    if (ring.empty())
    {
        cout << "No nodes available. Add a node first.\n";
        return;
    }

    NodeID keyID = hashKey(key);
    auto owner = successorOf(keyID);

    if (owner == ring.end())
        return;

    owner->second->store(key, value);

    replicateKey(key, value);

    cout << "Data stored successfully.\n";
    cout << "Responsible node: " << owner->first << "\n";
}

void DHT::get(const string& key) const
{
    string value;
    NodeID ownerID;

    if (getValue(key, value, ownerID))
    {
        cout << "Key: " << key << "\n";
        cout << "Value: " << value << "\n";
        cout << "Stored at node: " << ownerID << "\n";
    }
    else
    {
        cout << "Key not found.\n";
    }
}

void DHT::remove(const string& key)
{
    lock_guard<mutex> lock(ringMutex);

    if (ring.empty())
    {
        cout << "No nodes available.\n";
        return;
    }

    NodeID keyID = hashKey(key);
    auto owner = successorOf(keyID);

    if (owner == ring.end())
        return;

    bool removed = owner->second->erase(key);

    for (auto& replica : replicas)
    {
        replica.second.erase(key);
    }

    if (removed)
        cout << "Data deleted successfully.\n";
    else
        cout << "Key not found.\n";
}

void DHT::showResponsibleNode(const string& key) const
{
    lock_guard<mutex> lock(ringMutex);

    if (ring.empty())
    {
        cout << "No nodes available.\n";
        return;
    }

    NodeID keyID = hashKey(key);
    auto owner = successorOf(keyID);

    if (owner != ring.end())
    {
        cout << "Key: " << key << "\n";
        cout << "Key hash: " << keyID << "\n";
        cout << "Responsible node: " << owner->first << "\n";
    }
}

void DHT::displayNodes() const
{
    lock_guard<mutex> lock(ringMutex);

    if (ring.empty())
    {
        cout << "No nodes in the DHT.\n";
        return;
    }

    cout << "\n--- DHT Nodes ---\n";

    for (const auto& node : ring)
    {
        cout << "Node ID: " << node.first
             << " | Stored keys: " << node.second->dataCount()
             << "\n";
    }
}

void DHT::displayData() const
{
    lock_guard<mutex> lock(ringMutex);

    if (ring.empty())
    {
        cout << "No nodes available.\n";
        return;
    }

    cout << "\n--- DHT Data ---\n";

    bool found = false;

    for (const auto& node : ring)
    {
        auto entries = node.second->entries();

        for (const auto& entry : entries)
        {
            found = true;

            cout << "Node: " << node.first
                 << " | Key: " << entry.first
                 << " | Value: " << entry.second
                 << "\n";
        }
    }

    if (!found)
        cout << "No data stored.\n";
}

size_t DHT::nodeCount() const
{
    lock_guard<mutex> lock(ringMutex);
    return ring.size();
}

bool DHT::getValue(
    const string& key,
    string& valueOut,
    NodeID& ownerOut
) const
{
    lock_guard<mutex> lock(ringMutex);

    if (ring.empty())
        return false;

    NodeID keyID = hashKey(key);
    auto owner = successorOf(keyID);

    if (owner == ring.end())
        return false;

    if (owner->second->fetch(key, valueOut))
    {
        ownerOut = owner->first;
        return true;
    }

    return false;
}

vector<pair<NodeID, size_t>> DHT::nodesSnapshot() const
{
    lock_guard<mutex> lock(ringMutex);

    vector<pair<NodeID, size_t>> result;

    for (const auto& node : ring)
    {
        result.emplace_back(
            node.first,
            node.second->dataCount()
        );
    }

    return result;
}

vector<tuple<string, string, NodeID>> DHT::dataSnapshot() const
{
    lock_guard<mutex> lock(ringMutex);

    vector<tuple<string, string, NodeID>> result;

    for (const auto& node : ring)
    {
        auto entries = node.second->entries();

        for (const auto& entry : entries)
        {
            result.emplace_back(
                entry.first,
                entry.second,
                node.first
            );
        }
    }

    return result;
}

void DHT::replicateKey(
    const string& key,
    const string& value
)
{
    if (ring.size() < 2)
        return;

    NodeID keyID = hashKey(key);
    auto primary = successorOf(keyID);

    if (primary == ring.end())
        return;

    auto backup = next(primary);

    if (backup == ring.end())
        backup = ring.begin();

    if (backup->first == primary->first)
        return;

    replicas[backup->first][key] = value;
}

void DHT::rebuildReplicas()
{
    replicas.clear();

    if (ring.size() < 2)
        return;

    for (const auto& node : ring)
    {
        auto entries = node.second->entries();

        for (const auto& entry : entries)
        {
            replicateKey(entry.first, entry.second);
        }
    }
}

void DHT::displayReplicas() const
{
    lock_guard<mutex> lock(ringMutex);

    cout << "\n--- DHT Replica Data ---\n";

    if (replicas.empty())
    {
        cout << "No replica data available.\n";
        return;
    }

    bool found = false;

    for (const auto& node : replicas)
    {
        for (const auto& entry : node.second)
        {
            found = true;

            cout << "Backup Node: " << node.first
                 << " | Key: " << entry.first
                 << " | Value: " << entry.second
                 << "\n";
        }
    }

    if (!found)
        cout << "No replica data available.\n";
}


bool DHT::simulateNodeFailure(NodeID nodeID)
{
    lock_guard<mutex> lock(ringMutex);

    auto failedNode = ring.find(nodeID);

    if (failedNode == ring.end())
    {
        cout << "Node not found.\n";
        return false;
    }

    if (ring.size() < 2)
    {
        cout << "Failure simulation requires at least two nodes.\n";
        cout << "No backup node is available for recovery.\n";
        return false;
    }

    auto successor = next(failedNode);

    if (successor == ring.end())
        successor = ring.begin();

    NodeID successorID = successor->first;

    size_t expectedKeys = failedNode->second->dataCount();
    size_t recoveredKeys = 0;

    auto backupIt = replicas.find(successorID);

    if (backupIt != replicas.end())
    {
        for (const auto& entry : backupIt->second)
        {
            NodeID keyID = hashKey(entry.first);
            auto originalOwner = successorOf(keyID);

            if (originalOwner != ring.end() &&
                originalOwner->first == nodeID)
            {
                successor->second->store(entry.first, entry.second);
                recoveredKeys++;
            }
        }
    }

    ring.erase(failedNode);

    rebuildReplicas();

    cout << "\n--- Node Failure Simulation ---\n";
    cout << "Failed Node: " << nodeID << "\n";
    cout << "Recovery Node: " << successorID << "\n";
    cout << "Keys before failure: " << expectedKeys << "\n";
    cout << "Keys recovered: " << recoveredKeys << "\n";

    if (recoveredKeys == expectedKeys)
    {
        cout << "Recovery Status: SUCCESS\n";
    }
    else
    {
        cout << "Recovery Status: PARTIAL / DATA LOSS\n";
        cout << "Some keys did not have available backup copies.\n";
    }

    cout << "Node " << nodeID << " has been removed from the ring.\n";

    return recoveredKeys == expectedKeys;
}
