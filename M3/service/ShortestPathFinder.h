#pragma once

#include "structure/RoadGraph.h"
#include "dto/RouteResponse.h"
#include "dto/ApiResponse.h"

#include <string>

class ShortestPathFinder {
private:
    const RoadGraph& graph;

public:
    explicit ShortestPathFinder(const RoadGraph& graph);

    ApiResponse<RouteResponse> findShortestRoute(const std::string& startNodeId,
                                                const std::string& endNodeId) const;
};
