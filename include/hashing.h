#ifndef HASHING_H
#define HASHING_H

#include <cstdint>
#include <string>

using NodeID = std::uint64_t;

inline NodeID hashString(const std::string& text)
{
    NodeID hash = 14695981039346656037ULL;

    for (unsigned char c : text)
    {
        hash ^= c;
        hash *= 1099511628211ULL;
    }

    return hash;
}

inline NodeID hashNode(const std::string& ip, std::uint16_t port)
{
    return hashString(ip + ":" + std::to_string(port));
}

#endif
