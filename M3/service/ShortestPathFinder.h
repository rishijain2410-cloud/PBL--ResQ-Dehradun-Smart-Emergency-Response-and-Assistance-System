#ifndef M3_SERVICE_SHORTEST_PATH_FINDER_H
#define M3_SERVICE_SHORTEST_PATH_FINDER_H

#include "../structure/RoadGraph.h"
#include "../dto/ApiResponse.h"
#include "../dto/RouteResponse.h"
#include <memory>
#include <string>

class ShortestPathFinder {
private:
    std::shared_ptr<RoadGraph> graph;

public:
    explicit ShortestPathFinder(std::shared_ptr<RoadGraph> roadGraph);

    // Primary pathfinding engine using optimized Min-Heap Dijkstra
    ApiResponse<RouteResponse> findShortestRoute(const std::string& startNodeId,
                                                const std::string& endNodeId);

    // Dynamic Programming Bellman-Ford algorithm used for 100% path validation in unit test suite
    ApiResponse<RouteResponse> findShortestRouteBellmanFord(const std::string& startNodeId,
                                                           const std::string& endNodeId);
};

#endif // M3_SERVICE_SHORTEST_PATH_FINDER_H
