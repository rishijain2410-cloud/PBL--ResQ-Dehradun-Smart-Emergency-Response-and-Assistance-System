#include "ShortestPathFinder.h"
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>

ShortestPathFinder::ShortestPathFinder(std::shared_ptr<RoadGraph> roadGraph)
    : graph(roadGraph) {}

ApiResponse<RouteResponse> ShortestPathFinder::findShortestRoute(
    const std::string& startNodeId,
    const std::string& endNodeId) {

    // 1. Validation checks
    if (!graph) {
        return ApiResponse<RouteResponse>::error("INTERNAL_ERROR", "Road graph is null");
    }
    if (!graph->hasNode(startNodeId)) {
        return ApiResponse<RouteResponse>::error("NOT_FOUND", "Start location not found: " + startNodeId);
    }
    if (!graph->hasNode(endNodeId)) {
        return ApiResponse<RouteResponse>::error("NOT_FOUND", "Destination location not found: " + endNodeId);
    }

    // 2. Identity case: start equals destination
    if (startNodeId == endNodeId) {
        std::vector<std::string> path = { startNodeId };
        std::vector<RouteSegment> emptySegs;
        RouteResponse resp(startNodeId, endNodeId, 0.0, path, emptySegs, 0, "Dijkstra (Min-Heap)");
        return ApiResponse<RouteResponse>::ok(resp, "Origin and destination are identical");
    }

    // 3. Dijkstra structures
    const double INF = std::numeric_limits<double>::infinity();
    std::unordered_map<std::string, double> distanceMap;
    std::unordered_map<std::string, std::string> parentNodeMap;
    std::unordered_map<std::string, RoadEdge> parentEdgeMap;

    for (const auto& kv : graph->getAllNodes()) {
        distanceMap[kv.first] = INF;
    }
    distanceMap[startNodeId] = 0.0;

    // Min-heap storing pair: (tentativeDistance, nodeId)
    using HeapElement = std::pair<double, std::string>;
    std::priority_queue<HeapElement, std::vector<HeapElement>, std::greater<HeapElement>> minHeap;

    minHeap.push({0.0, startNodeId});
    int nodesExplored = 0;
    bool found = false;

    while (!minHeap.empty()) {
        auto [currentDist, u] = minHeap.top();
        minHeap.pop();
        nodesExplored++;

        // Stale entry check
        if (currentDist > distanceMap[u]) {
            continue;
        }

        // Goal-directed early termination
        if (u == endNodeId) {
            found = true;
            break;
        }

        for (const auto& edge : graph->getNeighbors(u)) {
            const std::string& v = edge.getToNodeId();
            double weight = edge.getDistanceKm();
            double newDist = currentDist + weight;

            if (newDist < distanceMap[v]) {
                distanceMap[v] = newDist;
                parentNodeMap[v] = u;
                parentEdgeMap[v] = edge;
                minHeap.push({newDist, v});
            }
        }
    }

    if (!found || distanceMap[endNodeId] == INF) {
        return ApiResponse<RouteResponse>::error("NO_RESULTS", "No connecting route found between locations");
    }

    // 4. Backtrack path reconstruction
    std::vector<std::string> pathNodes;
    std::vector<RouteSegment> segments;

    std::string curr = endNodeId;
    while (curr != startNodeId) {
        pathNodes.push_back(curr);
        const RoadEdge& edge = parentEdgeMap[curr];
        segments.emplace_back(edge.getFromNodeId(), edge.getToNodeId(),
                              edge.getRoadName(), edge.getRoadType(), edge.getDistanceKm());
        curr = parentNodeMap[curr];
    }
    pathNodes.push_back(startNodeId);

    std::reverse(pathNodes.begin(), pathNodes.end());
    std::reverse(segments.begin(), segments.end());

    RouteResponse response(startNodeId, endNodeId, distanceMap[endNodeId],
                          pathNodes, segments, nodesExplored, "Dijkstra (Min-Heap)");

    return ApiResponse<RouteResponse>::ok(response, "Shortest route calculated successfully");
}

ApiResponse<RouteResponse> ShortestPathFinder::findShortestRouteBellmanFord(
    const std::string& startNodeId,
    const std::string& endNodeId) {

    if (!graph || !graph->hasNode(startNodeId) || !graph->hasNode(endNodeId)) {
        return ApiResponse<RouteResponse>::error("NOT_FOUND", "Invalid start or destination node");
    }

    if (startNodeId == endNodeId) {
        std::vector<std::string> path = { startNodeId };
        RouteResponse resp(startNodeId, endNodeId, 0.0, path, {}, 0, "Bellman-Ford");
        return ApiResponse<RouteResponse>::ok(resp, "Identity route");
    }

    const double INF = std::numeric_limits<double>::infinity();
    std::unordered_map<std::string, double> dist;
    std::unordered_map<std::string, std::string> parentNode;
    std::unordered_map<std::string, RoadEdge> parentEdge;

    for (const auto& kv : graph->getAllNodes()) {
        dist[kv.first] = INF;
    }
    dist[startNodeId] = 0.0;

    const auto& edges = graph->getAllEdges();
    size_t V = graph->getNodeCount();

    // Relax all edges |V| - 1 times
    for (size_t i = 1; i < V; ++i) {
        bool changed = false;
        for (const auto& edge : edges) {
            const std::string& u = edge.getFromNodeId();
            const std::string& v = edge.getToNodeId();
            double w = edge.getDistanceKm();

            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parentNode[v] = u;
                parentEdge[v] = edge;
                changed = true;
            }
        }
        if (!changed) break; // Optimization
    }

    if (dist[endNodeId] == INF) {
        return ApiResponse<RouteResponse>::error("NO_RESULTS", "Unreachable");
    }

    std::vector<std::string> pathNodes;
    std::vector<RouteSegment> segments;
    std::string curr = endNodeId;

    while (curr != startNodeId) {
        pathNodes.push_back(curr);
        const RoadEdge& edge = parentEdge[curr];
        segments.emplace_back(edge.getFromNodeId(), edge.getToNodeId(),
                              edge.getRoadName(), edge.getRoadType(), edge.getDistanceKm());
        curr = parentNode[curr];
    }
    pathNodes.push_back(startNodeId);

    std::reverse(pathNodes.begin(), pathNodes.end());
    std::reverse(segments.begin(), segments.end());

    RouteResponse response(startNodeId, endNodeId, dist[endNodeId],
                          pathNodes, segments, static_cast<int>(V), "Bellman-Ford");
    return ApiResponse<RouteResponse>::ok(response, "Bellman-Ford path computed");
}
