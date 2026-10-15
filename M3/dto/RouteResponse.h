#pragma once

#include "dto/RouteSegment.h"
#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>

class RouteResponse {
private:
    std::string startNodeId;
    std::string endNodeId;
    double totalDistanceKm;
    std::vector<std::string> pathNodeIds;
    std::vector<RouteSegment> segments;
    std::size_t nodesExplored;

public:
    RouteResponse();

    RouteResponse(const std::string& startNodeId,
                  const std::string& endNodeId,
                  double totalDistanceKm,
                  const std::vector<std::string>& pathNodeIds,
                  const std::vector<RouteSegment>& segments,
                  std::size_t nodesExplored);

    const std::string& getStartNodeId() const;
    const std::string& getEndNodeId() const;
    double getTotalDistanceKm() const;
    const std::vector<std::string>& getPathNodeIds() const;
    const std::vector<RouteSegment>& getSegments() const;
    std::size_t getNodesExplored() const;

    std::size_t segmentCount() const;

    std::string toString() const;
};
