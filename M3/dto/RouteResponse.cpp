#include "RouteResponse.h"
#include <sstream>
#include <iomanip>

RouteResponse::RouteResponse()
    : startNodeId(""), endNodeId(""), totalDistanceKm(0.0),
      nodesExplored(0), algorithm("Dijkstra (Min-Heap)") {}

RouteResponse::RouteResponse(const std::string& start,
                             const std::string& end,
                             double distance,
                             const std::vector<std::string>& pathNodes,
                             const std::vector<RouteSegment>& segs,
                             int explored,
                             const std::string& algo)
    : startNodeId(start), endNodeId(end), totalDistanceKm(distance),
      pathNodeIds(pathNodes), segments(segs),
      nodesExplored(explored), algorithm(algo) {}

void RouteResponse::addTravelEstimate(const std::string& mode, double minutes, double cost) {
    travelTimeMinutes[mode] = minutes;
    estimatedCost[mode] = cost;
}

std::string RouteResponse::toJson() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{"
       << "\"startNodeId\":\"" << startNodeId << "\","
       << "\"endNodeId\":\"" << endNodeId << "\","
       << "\"totalDistanceKm\":" << totalDistanceKm << ","
       << "\"nodesExplored\":" << nodesExplored << ","
       << "\"algorithm\":\"" << algorithm << "\","
       << "\"pathNodeIds\":[";

    for (size_t i = 0; i < pathNodeIds.size(); ++i) {
        ss << "\"" << pathNodeIds[i] << "\"";
        if (i + 1 < pathNodeIds.size()) ss << ",";
    }
    ss << "],";

    ss << "\"segments\":[";
    for (size_t i = 0; i < segments.size(); ++i) {
        ss << segments[i].toJson();
        if (i + 1 < segments.size()) ss << ",";
    }
    ss << "],";

    ss << "\"travelEstimates\":{";
    size_t count = 0;
    for (const auto& kv : travelTimeMinutes) {
        double cost = 0.0;
        auto costIt = estimatedCost.find(kv.first);
        if (costIt != estimatedCost.end()) cost = costIt->second;

        ss << "\"" << kv.first << "\":{"
           << "\"durationMinutes\":" << kv.second << ","
           << "\"estimatedCostRupees\":" << cost
           << "}";
        if (++count < travelTimeMinutes.size()) ss << ",";
    }
    ss << "}}";

    return ss.str();
}
