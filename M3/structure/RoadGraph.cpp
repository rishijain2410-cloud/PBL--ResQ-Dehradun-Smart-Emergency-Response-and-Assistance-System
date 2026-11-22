#include "RoadGraph.h"
#include "../util/GeoUtil.h"
#include <queue>
#include <unordered_set>
#include <limits>
#include <stdexcept>

RoadGraph::RoadGraph() {}

bool RoadGraph::addNode(const LocationNode& node) {
    const std::string& id = node.getId();
    if (nodes.find(id) != nodes.end()) {
        return false; // Already present
    }
    nodes[id] = node;
    adjacencyList[id] = std::vector<RoadEdge>();
    return true;
}

bool RoadGraph::hasNode(const std::string& nodeId) const {
    return nodes.find(nodeId) != nodes.end();
}

const LocationNode& RoadGraph::getNode(const std::string& nodeId) const {
    auto it = nodes.find(nodeId);
    if (it == nodes.end()) {
        throw std::out_of_range("Node not found in graph: " + nodeId);
    }
    return it->second;
}

void RoadGraph::addEdge(const RoadEdge& edge, bool bidirectional) {
    if (!hasNode(edge.getFromNodeId()) || !hasNode(edge.getToNodeId())) {
        throw std::invalid_argument("Cannot add edge: one or both endpoints do not exist in graph ("
                                    + edge.getFromNodeId() + " -> " + edge.getToNodeId() + ")");
    }

    adjacencyList[edge.getFromNodeId()].push_back(edge);
    allEdges.push_back(edge);

    if (bidirectional) {
        RoadEdge rev = edge.getReverseEdge();
        adjacencyList[edge.getToNodeId()].push_back(rev);
        allEdges.push_back(rev);
    }
}

static const std::vector<RoadEdge> EMPTY_EDGES;

const std::vector<RoadEdge>& RoadGraph::getNeighbors(const std::string& nodeId) const {
    auto it = adjacencyList.find(nodeId);
    if (it == adjacencyList.end()) {
        return EMPTY_EDGES;
    }
    return it->second;
}

bool RoadGraph::isConnected() const {
    if (nodes.empty()) return true;
    if (nodes.size() == 1) return true;

    // Pick first node as source
    std::string startNode = nodes.begin()->first;
    std::unordered_set<std::string> visited;
    std::queue<std::string> q;

    q.push(startNode);
    visited.insert(startNode);

    while (!q.empty()) {
        std::string curr = q.front();
        q.pop();

        for (const auto& edge : getNeighbors(curr)) {
            const std::string& nextNode = edge.getToNodeId();
            if (visited.find(nextNode) == visited.end()) {
                visited.insert(nextNode);
                q.push(nextNode);
            }
        }
    }

    return visited.size() == nodes.size();
}

std::string RoadGraph::findNearestNodeId(double lat, double lon) const {
    if (nodes.empty()) {
        return "";
    }

    std::string nearestId = "";
    double minDistance = std::numeric_limits<double>::max();

    for (const auto& kv : nodes) {
        const LocationNode& node = kv.second;
        double dist = GeoUtil::haversineDistanceKm(lat, lon, node.getLatitude(), node.getLongitude());
        if (dist < minDistance) {
            minDistance = dist;
            nearestId = kv.first;
        }
    }

    return nearestId;
}

void RoadGraph::clear() {
    nodes.clear();
    adjacencyList.clear();
    allEdges.clear();
}
