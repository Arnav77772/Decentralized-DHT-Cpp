#ifndef DASHBOARD_SERVER_H
#define DASHBOARD_SERVER_H

#include <atomic>
#include <thread>

class DHT;

class DashboardServer
{
public:
    explicit DashboardServer(DHT& dht);
    ~DashboardServer();

    bool start(unsigned short port = 8080);
    void stop();

private:
    void run();

    DHT& dht_;
    std::atomic<bool> running_{false};
    int listenSocket_ = -1;
    std::thread worker_;
};

#endif