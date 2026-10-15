#include "service/ShortestPathFinder.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <limits>

using namespace std;

ShortestPathFinder::ShortestPathFinder(const RoadGraph& graph)
    : graph(graph) {}

ApiResponse<RouteResponse> ShortestPathFinder::findShortestRoute(
    const string& startNodeId,
    const string& endNodeId) const {

    if (startNodeId.empty() || endNodeId.empty()) {
        return ApiResponse<RouteResponse>::fail(
            "Validation Error: Start and destination node IDs cannot be empty.",
            ErrorCode::INVALID_ID
        );
    }

    if (!graph.hasNode(startNodeId)) {
        return ApiResponse<RouteResponse>::fail(
            "Start location '" + startNodeId + "' does not exist in the road network graph.",
            ErrorCode::NOT_FOUND
        );
    }
    if (!graph.hasNode(endNodeId)) {
        return ApiResponse<RouteResponse>::fail(
            "Destination location '" + endNodeId + "' does not exist in the road network graph.",
            ErrorCode::NOT_FOUND
        );
    }

    if (startNodeId == endNodeId) {
        RouteResponse zeroRoute(
            startNodeId,
            endNodeId,
            0.0,
            {startNodeId},
            {},
            1
        );
        return ApiResponse<RouteResponse>::ok(
            zeroRoute,
            "Start and destination are identical."
        );
    }

    using QueueElement = pair<double, string>;
    priority_queue<QueueElement, vector<QueueElement>, greater<QueueElement>> minHeap;

    unordered_map<string, double> distanceMap;
    unordered_map<string, string> parentNodeMap;
    unordered_map<string, RoadEdge> parentEdgeMap;
    unordered_set<string> visitedSet;

    distanceMap[startNodeId] = 0.0;
    minHeap.emplace(0.0, startNodeId);

    size_t nodesExplored = 0;
    bool reachedDestination = false;

    while (!minHeap.empty()) {
        double currentDist = minHeap.top().first;
        string currentId = minHeap.top().second;
        minHeap.pop();

        if (visitedSet.find(currentId) != visitedSet.end()) {
            continue;
        }

        visitedSet.insert(currentId);
        nodesExplored++;

        if (currentId == endNodeId) {
            reachedDestination = true;
            break;
        }

        for (const auto& edge : graph.getNeighbors(currentId)) {
            const string& neighborId = edge.getToNodeId();

            if (visitedSet.find(neighborId) != visitedSet.end()) {
                continue;
            }

            double newDist = currentDist + edge.getDistanceKm();
            auto distIt = distanceMap.find(neighborId);

            if (distIt == distanceMap.end() || newDist < distIt->second) {
                distanceMap[neighborId] = newDist;
                parentNodeMap[neighborId] = currentId;
                parentEdgeMap[neighborId] = edge;
                minHeap.emplace(newDist, neighborId);
            }
        }
    }

    if (!reachedDestination) {
        return ApiResponse<RouteResponse>::fail(
            "No route found between '" + startNodeId + "' and '" + endNodeId + "'. Locations are disconnected.",
            ErrorCode::NO_RESULTS
        );
    }

    vector<string> pathNodeIds;
    vector<RouteSegment> segments;

    string current = endNodeId;
    while (current != startNodeId) {
        pathNodeIds.push_back(current);
        const RoadEdge& edge = parentEdgeMap.at(current);
        segments.emplace_back(
            edge.getFromNodeId(),
            edge.getToNodeId(),
            edge.getRoadName(),
            edge.getDistanceKm(),
            edge.getRoadType()
        );
        current = parentNodeMap.at(current);
    }
    pathNodeIds.push_back(startNodeId);

    reverse(pathNodeIds.begin(), pathNodeIds.end());
    reverse(segments.begin(), segments.end());

    double totalDistanceKm = distanceMap.at(endNodeId);
    RouteResponse response(
        startNodeId,
        endNodeId,
        totalDistanceKm,
        pathNodeIds,
        segments,
        nodesExplored
    );

    return ApiResponse<RouteResponse>::ok(
        response,
        "Shortest route computed successfully."
    );
}
