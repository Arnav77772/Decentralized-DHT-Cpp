# Distributed Hash Table Using C and C++ Data Structures

## Project Title

**Design and Implementation of a Decentralized Distributed Hash Table Using C and C++ Data Structures for Key-Value Storage and Retrieval**

---

## Team Information

- **Team Name:** Hash Titans
- **Team ID:** T396
- **Department:** Computer Science & Engineering
- **University:** Graphic Era (Deemed to be University), Dehradun
- **Academic Session:** 2026–27
- **Project Phase:** Phase I – Initial Design and Planning

### Team Members

| Name | Roll Number | Responsibility |
|---|---|---|
| Garima Thakur | 2510380093 | DHT Design & Architecture |
| Arnav Singh | 2510370502 | C++ Development |
| Yash Rawat | 2510376591 | Data Structures & Testing |

---

## 1. Project Overview

A **Distributed Hash Table (DHT)** is a decentralized data structure used to store and retrieve data across multiple nodes without depending on a single central server.

The proposed project aims to design and develop a basic DHT prototype using **C and C++ data structures**. The system will store data in the form of **key-value pairs** and determine the appropriate node for each key using hashing.

The project focuses on demonstrating how data structures such as hashing, arrays, linked lists, trees, graphs, queues, and other supporting structures can be integrated into a distributed data-management system.

---

## 2. Problem Statement

Traditional centralized storage systems depend heavily on a central server. As the number of users and requests increases, the central server may become a bottleneck and can represent a single point of failure.

The proposed system addresses this concept by distributing key-value data among multiple simulated nodes. A hashing mechanism will determine where each key should be stored, allowing data to be distributed across the network.

The system will also study how data can be redistributed when nodes join or leave the network.

---

## 3. Motivation

The main motivation of this project is to understand the implementation of decentralized data storage using fundamental data structures.

The project focuses on:

- Reducing dependency on a centralized storage server.
- Distributing data across multiple nodes.
- Providing deterministic key-to-node mapping.
- Understanding hashing and collision handling.
- Supporting dynamic node addition and removal.
- Minimizing unnecessary data movement.
- Studying efficient lookup operations.
- Applying concepts from Data Structures and C/C++ programming.

---

## 4. Objectives

The objectives of Phase I and the overall project are:

1. Design the architecture of a basic Distributed Hash Table.
2. Implement deterministic hashing for keys and nodes.
3. Support basic operations such as:
   - `PUT`
   - `GET`
   - `DELETE`
4. Integrate important data structures into the system.
5. Design node join and node removal mechanisms.
6. Study data redistribution after changes in the node network.
7. Simulate node failures and recovery.
8. Evaluate lookup efficiency and data movement.
9. Develop a working prototype using C/C++.

---

## 5. Proposed System

The proposed system will consist of multiple simulated nodes.

When a user provides a key-value pair:

```text
Key → Hash Function → DHT Routing → Responsible Node → Storagen
```

## 6. Updates Made

There are changes made in the proposed system and they are as follows:

```text
Garima : Created the main.cpp file,updated the code base for main.cpp,created node.h,DHT.h,Hashtable.h
```

```text
Yash : Updated the readMe intially and moving forward updated the Node.h and Hashtable.h
```

```text
Arnav : Implemented the core DHT ring logic, node join/leave operations, key hashing, data redistribution, and tested the code.
```
