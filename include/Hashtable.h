#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>


struct KeyValue
{
    std::string key;
    std::string value;
    KeyValue*   next;

    KeyValue(const std::string& k, const std::string& v)
        : key(k), value(v), next(nullptr)
    {
    }
};


class HashTable
{
public:
    static const int TABLE_SIZE = 10;   

    HashTable();
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    bool put(const std::string& key, const std::string& value);

    bool get(const std::string& key, std::string& valueOut) const;

    bool remove(const std::string& key);

    void clear();

    std::size_t size() const;

    std::vector<std::pair<std::string, std::string>> entries() const;

    void display() const;

private:
    KeyValue*   buckets[TABLE_SIZE];
    std::size_t count;

    int bucketIndex(const std::string& key) const;
};

#endif
