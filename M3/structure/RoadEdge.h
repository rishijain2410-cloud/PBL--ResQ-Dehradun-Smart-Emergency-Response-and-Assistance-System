#pragma once

#include <string>
#include <stdexcept>

enum class RoadType {
    HIGHWAY,
    MAIN_ROAD,
    CITY_STREET,
    HILL_ROAD
};

class RoadEdge {
private:
    std::string edgeId;
    std::string fromNodeId;
    std::string toNodeId;
    std::string roadName;
    double distanceKm;
    RoadType roadType;

public:
    RoadEdge();

    RoadEdge(const std::string& edgeId,
             const std::string& fromNodeId,
             const std::string& toNodeId,
             const std::string& roadName,
             double distanceKm,
             RoadType roadType);

    const std::string& getEdgeId() const;
    const std::string& getFromNodeId() const;
    const std::string& getToNodeId() const;
    const std::string& getRoadName() const;
    double getDistanceKm() const;
    RoadType getRoadType() const;

    std::string toString() const;

    static std::string roadTypeToString(RoadType type);
    static RoadType stringToRoadType(const std::string& typeStr);

    bool operator==(const RoadEdge& other) const;
    bool operator!=(const RoadEdge& other) const;
};
