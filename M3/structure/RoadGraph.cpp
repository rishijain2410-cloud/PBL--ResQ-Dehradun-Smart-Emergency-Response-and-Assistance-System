#include "structure/RoadGraph.h"
#include "util/GeoUtil.h"

#include <queue>
#include <limits>
#include <stdexcept>

using namespace std;

void RoadGraph::addNode(const LocationNode& node) {
    const string& id = node.getNodeId();
    if (itemMapById.find(id) != itemMapById.end()) {
        throw invalid_argument(
            "Validation Error: Node with ID '" + id + "' already exists in RoadGraph."
        );
    }
    itemMapById.emplace(id, node);
    adjacencyList[id] = {}; // Ensure empty adjacency entry is initialized
}

void RoadGraph::addEdge(const RoadEdge& edge, bool bidirectional) {
    const string& fromId = edge.getFromNodeId();
    const string& toId = edge.getToNodeId();
    const string& edgeId = edge.getEdgeId();

    // Verify both endpoints exist
    if (!hasNode(fromId)) {
        throw invalid_argument(
            "Cannot add edge '" + edgeId + "': From-node '" + fromId + "' does not exist in graph."
        );
    }
    if (!hasNode(toId)) {
        throw invalid_argument(
            "Cannot add edge '" + edgeId + "': To-node '" + toId + "' does not exist in graph."
        );
    }

    // Verify edge ID uniqueness
    if (uniqueIdSet.find(edgeId) != uniqueIdSet.end()) {
        throw invalid_argument(
            "Validation Error: Duplicate edge ID '" + edgeId + "' rejected."
        );
    }

    uniqueIdSet.insert(edgeId);

    // Register forward edge
    adjacencyList[fromId].push_back(edge);

    // Register reverse edge if bidirectional
    if (bidirectional) {
        RoadEdge reverseEdge(
            edgeId,
            toId,
            fromId,
            edge.getRoadName(),
            edge.getDistanceKm(),
            edge.getRoadType()
        );
        adjacencyList[toId].push_back(reverseEdge);
    }
}

bool RoadGraph::hasNode(const string& nodeId) const {
    return itemMapById.find(nodeId) != itemMapById.end();
}

const LocationNode& RoadGraph::getNode(const string& nodeId) const {
    auto it = itemMapById.find(nodeId);
    if (it == itemMapById.end()) {
        throw out_of_range(
            "LocationNode with ID '" + nodeId + "' was not found in RoadGraph."
        );
    }
    return it->second;
}

const vector<RoadEdge>& RoadGraph::getNeighbors(const string& nodeId) const {
    if (!hasNode(nodeId)) {
        throw out_of_range(
            "Cannot get neighbors: LocationNode with ID '" + nodeId + "' was not found in RoadGraph."
        );
    }
    return adjacencyList.at(nodeId);
}

size_t RoadGraph::nodeCount() const {
    return itemMapById.size();
}

size_t RoadGraph::edgeCount() const {
    return uniqueIdSet.size();
}

string RoadGraph::findNearestNodeId(double lat, double lon) const {
    if (itemMapById.empty()) {
        throw runtime_error("Cannot find nearest node: RoadGraph contains no nodes.");
    }

    string nearestNodeId = "";
    double minDistanceKm = numeric_limits<double>::infinity();

    for (const auto& entry : itemMapById) {
        const LocationNode& node = entry.second;
        double dist = GeoUtil::haversineDistanceKm(
            lat, lon,
            node.getLatitude(), node.getLongitude()
        );

        if (dist < minDistanceKm) {
            minDistanceKm = dist;
            nearestNodeId = node.getNodeId();
        }
    }

    return nearestNodeId;
}

bool RoadGraph::isConnected() const {
    // Vacuously connected for 0 or 1 node
    if (itemMapById.size() <= 1) {
        return true;
    }

    // Canonical queue variable name from 03_SHARED_OOP_DSA_STANDARD.md
    queue<string> requestQueue;
    unordered_set<string> visited;

    string startNodeId = itemMapById.begin()->first;
    visited.insert(startNodeId);
    requestQueue.push(startNodeId);

    // BFS Traversal
    while (!requestQueue.empty()) {
        string currentId = requestQueue.front();
        requestQueue.pop();

        auto it = adjacencyList.find(currentId);
        if (it != adjacencyList.end()) {
            for (const auto& edge : it->second) {
                const string& neighborId = edge.getToNodeId();
                if (visited.find(neighborId) == visited.end()) {
                    visited.insert(neighborId);
                    requestQueue.push(neighborId);
                }
            }
        }
    }

    // Graph is connected if every registered vertex was reached by BFS
    return visited.size() == itemMapById.size();
}

vector<string> RoadGraph::getAllNodeIds() const {
    vector<string> ids;
    ids.reserve(itemMapById.size());
    for (const auto& pair : itemMapById) {
        ids.push_back(pair.first);
    }
    return ids;
}

void RoadGraph::clear() {
    itemMapById.clear();
    adjacencyList.clear();
    uniqueIdSet.clear();
}
