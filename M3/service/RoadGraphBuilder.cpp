#include "RoadGraphBuilder.h"
#include <iostream>

std::shared_ptr<RoadGraph> RoadGraphBuilder::buildGraph(
    const ILocationRepository& locationRepo,
    const std::vector<RoadEdge>& edges,
    bool bidirectional) {

    auto graph = std::make_shared<RoadGraph>();

    // 1. Ingest all locations as nodes
    std::vector<Location> locations = locationRepo.findAll();
    for (const auto& loc : locations) {
        LocationNode node(loc);
        graph->addNode(node);
    }

    // 2. Link all edges
    for (const auto& edge : edges) {
        if (graph->hasNode(edge.getFromNodeId()) && graph->hasNode(edge.getToNodeId())) {
            graph->addEdge(edge, bidirectional);
        } else {
            std::cerr << "[RoadGraphBuilder] Warning: Skipping edge with unknown endpoints: "
                      << edge.getFromNodeId() << " -> " << edge.getToNodeId() << std::endl;
        }
    }

    // 3. Connectivity validation
    if (!graph->isConnected()) {
        std::cerr << "[RoadGraphBuilder] Alert: Road network contains disconnected subgraphs!" << std::endl;
    }

    return graph;
}
