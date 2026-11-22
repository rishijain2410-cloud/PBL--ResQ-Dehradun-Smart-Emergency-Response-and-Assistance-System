#ifndef M3_DTO_ROUTE_RESPONSE_H
#define M3_DTO_ROUTE_RESPONSE_H

#include "RouteSegment.h"
#include <vector>
#include <string>
#include <map>

class RouteResponse {
private:
    std::string startNodeId;
    std::string endNodeId;
    double totalDistanceKm;
    std::vector<std::string> pathNodeIds;
    std::vector<RouteSegment> segments;
    int nodesExplored;
    std::string algorithm;
    std::map<std::string, double> travelTimeMinutes; // mode -> minutes
    std::map<std::string, double> estimatedCost;     // mode -> rupees

public:
    RouteResponse();
    RouteResponse(const std::string& start,
                  const std::string& end,
                  double distance,
                  const std::vector<std::string>& pathNodes,
                  const std::vector<RouteSegment>& segs,
                  int explored = 0,
                  const std::string& algo = "Dijkstra (Min-Heap)");

    const std::string& getStartNodeId() const { return startNodeId; }
    const std::string& getEndNodeId() const { return endNodeId; }
    double getTotalDistanceKm() const { return totalDistanceKm; }
    const std::vector<std::string>& getPathNodeIds() const { return pathNodeIds; }
    const std::vector<RouteSegment>& getSegments() const { return segments; }
    int getNodesExplored() const { return nodesExplored; }
    const std::string& getAlgorithm() const { return algorithm; }

    void addTravelEstimate(const std::string& mode, double minutes, double cost);
    const std::map<std::string, double>& getTravelTimes() const { return travelTimeMinutes; }
    const std::map<std::string, double>& getEstimatedCosts() const { return estimatedCost; }

    std::string toJson() const;
};

#endif // M3_DTO_ROUTE_RESPONSE_H
