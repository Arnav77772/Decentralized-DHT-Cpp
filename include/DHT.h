#ifndef DHT_H
#define DHT_H

#include <cstddef>
#include <map>
#include <memory>
#include <string>

#include "Node.h"


const int RING_SIZE = 100;


class DHT
{
public:
    using RingMap = std::map<int, std::unique_ptr<Node>>;

    DHT() = default;

    static int hashKey(const std::string& key);

    bool addNode(int nodeID);       
    bool removeNode(int nodeID);    

    void put(const std::string& key, const std::string& value);
    void get(const std::string& key) const;
    void remove(const std::string& key);

    void showResponsibleNode(const std::string& key) const;
    void displayNodes() const;
    void displayData() const;

    std::size_t nodeCount() const;

private:
    RingMap ring;


    RingMap::const_iterator successorOf(int position) const;

    
    RingMap::const_iterator predecessorOf(int nodeID) const;

    static bool inRange(int h, int from, int to);
};

#endif