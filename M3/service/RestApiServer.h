#pragma once

#include "service/ShortestPathFinder.h"
#include "service/NearbyServicesFinder.h"
#include "repository/LocationRepository.h"

#include <memory>
#include <thread>
#include <atomic>
#include <string>

namespace httplib {
    class Server;
}

class RestApiServer {
private:
    int port;
    const ShortestPathFinder& pathFinder;
    const NearbyServicesFinder& nearbyFinder;
    const ILocationRepository& repo;

    std::unique_ptr<httplib::Server> server;
    std::unique_ptr<std::thread> serverThread;
    std::atomic<bool> isRunning{false};

    void setupRoutes();

public:
    RestApiServer(int port,
                  const ShortestPathFinder& pathFinder,
                  const NearbyServicesFinder& nearbyFinder,
                  const ILocationRepository& repo);
    ~RestApiServer();

    void start();

    void stop();

    bool running() const { return isRunning.load(); }
    int getPort() const { return port; }
};
