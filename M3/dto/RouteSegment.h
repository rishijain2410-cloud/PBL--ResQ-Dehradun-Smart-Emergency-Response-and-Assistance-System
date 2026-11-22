#ifndef M3_DTO_ROUTE_SEGMENT_H
#define M3_DTO_ROUTE_SEGMENT_H

#include "../structure/RoadEdge.h"
#include <string>
#include <sstream>
#include <iomanip>

struct RouteSegment {
    std::string fromNodeId;
    std::string toNodeId;
    std::string roadName;
    RoadType roadType;
    double distanceKm;

    RouteSegment()
        : fromNodeId(""), toNodeId(""), roadName(""),
          roadType(RoadType::UNKNOWN), distanceKm(0.0) {}

    RouteSegment(const std::string& from,
                 const std::string& to,
                 const std::string& name,
                 RoadType type,
                 double dist)
        : fromNodeId(from), toNodeId(to), roadName(name),
          roadType(type), distanceKm(dist) {}

    std::string toJson() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        ss << "{"
           << "\"from\":\"" << fromNodeId << "\","
           << "\"to\":\"" << toNodeId << "\","
           << "\"roadName\":\"" << roadName << "\","
           << "\"roadType\":\"" << roadTypeToString(roadType) << "\","
           << "\"distanceKm\":" << distanceKm
           << "}";
        return ss.str();
    }
};

#endif // M3_DTO_ROUTE_SEGMENT_H
