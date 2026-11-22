#ifndef M3_STRUCTURE_ROAD_GRAPH_H
#define M3_STRUCTURE_ROAD_GRAPH_H

#include "LocationNode.h"
#include "RoadEdge.h"
#include <unordered_map>
#include <vector>
#include <string>

class RoadGraph {
private:
    std::unordered_map<std::string, LocationNode> nodes;
    std::unordered_map<std::string, std::vector<RoadEdge>> adjacencyList;
    std::vector<RoadEdge> allEdges;

public:
    RoadGraph();

    // Node operations
    bool addNode(const LocationNode& node);
    bool hasNode(const std::string& nodeId) const;
    const LocationNode& getNode(const std::string& nodeId) const;
    const std::unordered_map<std::string, LocationNode>& getAllNodes() const { return nodes; }
    size_t getNodeCount() const { return nodes.size(); }

    // Edge operations
    void addEdge(const RoadEdge& edge, bool bidirectional = true);
    const std::vector<RoadEdge>& getNeighbors(const std::string& nodeId) const;
    const std::vector<RoadEdge>& getAllEdges() const { return allEdges; }
    size_t getEdgeCount() const { return allEdges.size(); }

    // Algorithmic graph analysis
    bool isConnected() const;
    std::string findNearestNodeId(double lat, double lon) const;

    // Clears graph
    void clear();
};

#endif // M3_STRUCTURE_ROAD_GRAPH_H
