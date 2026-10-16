#include "service/RoadGraphBuilder.h"

#include <iostream>

using namespace std;

RoadGraph RoadGraphBuilder::build(const ILocationRepository& repo,
                                  const vector<RoadEdge>& edges) {
    RoadGraph graph;

    vector<Location> locations = repo.findAll();
    for (const auto& loc : locations) {
        try {
            graph.addNode(LocationNode::fromLocation(loc));
        } catch (const exception& ex) {
            cerr << "[RoadGraphBuilder Warning] Could not add node for location "
                      << loc.getLocationId() << ": " << ex.what() << "\n";
        }
    }

    for (const auto& edge : edges) {
        if (!graph.hasNode(edge.getFromNodeId()) || !graph.hasNode(edge.getToNodeId())) {
            cerr << "[RoadGraphBuilder Warning] Edge " << edge.getEdgeId()
                      << " references unknown location ('" << edge.getFromNodeId()
                      << "' -> '" << edge.getToNodeId() << "'). Skipping.\n";
            continue;
        }

        try {
            graph.addEdge(edge, true);
        } catch (const exception& ex) {
            cerr << "[RoadGraphBuilder Warning] Failed to register edge "
                      << edge.getEdgeId() << ": " << ex.what() << ". Skipping.\n";
        }
    }

    return graph;
}
