#pragma once

#include "structure/LocationNode.h"
#include "structure/RoadEdge.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>

class RoadGraph {
private:
    std::unordered_map<std::string, LocationNode> itemMapById;
    std::unordered_map<std::string, std::vector<RoadEdge>> adjacencyList;
    std::unordered_set<std::string> uniqueIdSet;

public:
    RoadGraph() = default;

    void addNode(const LocationNode& node);

    void addEdge(const RoadEdge& edge, bool bidirectional = true);

    bool hasNode(const std::string& nodeId) const;

    const LocationNode& getNode(const std::string& nodeId) const;

    const std::vector<RoadEdge>& getNeighbors(const std::string& nodeId) const;

    std::size_t nodeCount() const;

    std::size_t edgeCount() const;

    std::string findNearestNodeId(double lat, double lon) const;

    bool isConnected() const;

    std::vector<std::string> getAllNodeIds() const;

    void clear();
};
