#include "HashTable.h"


#include <iostream>


HashTable::HashTable() : count(0)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        buckets[i] = nullptr;
    }
}


HashTable::~HashTable()
{
    clear();
}



int HashTable::bucketIndex(const std::string& key) const
{
    unsigned int h = 5381;

    for (unsigned char c : key)
    {
        h = h * 33 + c;
    }

    return static_cast<int>(h % TABLE_SIZE);
}


bool HashTable::put(const std::string& key, const std::string& value)
{
    int index = bucketIndex(key);

    KeyValue* current = buckets[index];

    while (current != nullptr)
    {
        if (current->key == key)
        {
            current->value = value;
            return false;
        }

        current = current->next;
    }

    KeyValue* newItem = new KeyValue(key, value);
    newItem->next = buckets[index];
    buckets[index] = newItem;
    count++;

    return true;
}


bool HashTable::get(const std::string& key, std::string& valueOut) const
{
    int index = bucketIndex(key);

    const KeyValue* current = buckets[index];

    while (current != nullptr)
    {
        if (current->key == key)
        {
            valueOut = current->value;
            return true;
        }

        current = current->next;
    }

    return false;
}


bool HashTable::remove(const std::string& key)
{
    int index = bucketIndex(key);

    KeyValue* current = buckets[index];
    KeyValue* previous = nullptr;

    while (current != nullptr)
    {
        if (current->key == key)
        {
            if (previous == nullptr)
            {
                buckets[index] = current->next;     
            }
            else
            {
                previous->next = current->next;     
            }

            delete current;
            count--;
            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}


void HashTable::clear()
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        KeyValue* current = buckets[i];

        while (current != nullptr)
        {
            KeyValue* next = current->next;
            delete current;
            current = next;
        }

        buckets[i] = nullptr;
    }

    count = 0;
}


std::size_t HashTable::size() const
{
    return count;
}


std::vector<std::pair<std::string, std::string>> HashTable::entries() const
{
    std::vector<std::pair<std::string, std::string>> result;

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        const KeyValue* current = buckets[i];

        while (current != nullptr)
        {
            result.push_back({current->key, current->value});
            current = current->next;
        }
    }

    return result;
}


void HashTable::display() const
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        std::cout << "  Bucket " << i << ": ";

        const KeyValue* current = buckets[i];

        while (current != nullptr)
        {
            std::cout << "[" << current->key
                      << " : " << current->value << "] -> ";

            current = current->next;
        }

        std::cout << "NULL\n";
    }
}