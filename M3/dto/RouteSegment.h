#pragma once

#include "structure/RoadEdge.h"
#include <string>

class RouteSegment {
private:
    std::string fromNodeId;
    std::string toNodeId;
    std::string roadName;
    double distanceKm;
    RoadType roadType;

public:
    RouteSegment();

    RouteSegment(const std::string& fromNodeId,
                 const std::string& toNodeId,
                 const std::string& roadName,
                 double distanceKm,
                 RoadType roadType);

    const std::string& getFromNodeId() const;
    const std::string& getToNodeId() const;
    const std::string& getRoadName() const;
    double getDistanceKm() const;
    RoadType getRoadType() const;

    std::string toString() const;

    bool operator==(const RouteSegment& other) const;
    bool operator!=(const RouteSegment& other) const;
};
