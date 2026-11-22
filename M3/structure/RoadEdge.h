#ifndef M3_STRUCTURE_ROAD_EDGE_H
#define M3_STRUCTURE_ROAD_EDGE_H

#include <string>

enum class RoadType {
    CITY_STREET,
    MAIN_ROAD,
    HIGHWAY,
    HILL_ROAD,
    UNKNOWN
};

std::string roadTypeToString(RoadType type);
RoadType stringToRoadType(const std::string& typeStr);

class RoadEdge {
private:
    std::string fromNodeId;
    std::string toNodeId;
    double distanceKm;
    std::string roadName;
    RoadType roadType;

public:
    RoadEdge();
    RoadEdge(const std::string& from,
             const std::string& to,
             double distKm,
             const std::string& name,
             RoadType type);

    const std::string& getFromNodeId() const { return fromNodeId; }
    const std::string& getToNodeId() const { return toNodeId; }
    double getDistanceKm() const { return distanceKm; }
    const std::string& getRoadName() const { return roadName; }
    RoadType getRoadType() const { return roadType; }
    std::string getRoadTypeString() const { return roadTypeToString(roadType); }

    RoadEdge getReverseEdge() const {
        return RoadEdge(toNodeId, fromNodeId, distanceKm, roadName, roadType);
    }
};

#endif // M3_STRUCTURE_ROAD_EDGE_H
