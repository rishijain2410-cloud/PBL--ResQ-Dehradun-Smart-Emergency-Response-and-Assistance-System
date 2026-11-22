#include "RoadEdge.h"
#include <algorithm>

std::string roadTypeToString(RoadType type) {
    switch (type) {
        case RoadType::CITY_STREET: return "CITY_STREET";
        case RoadType::MAIN_ROAD: return "MAIN_ROAD";
        case RoadType::HIGHWAY: return "HIGHWAY";
        case RoadType::HILL_ROAD: return "HILL_ROAD";
        default: return "UNKNOWN";
    }
}

RoadType stringToRoadType(const std::string& typeStr) {
    std::string s = typeStr;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s == "CITY_STREET" || s == "STREET") return RoadType::CITY_STREET;
    if (s == "MAIN_ROAD" || s == "MAIN") return RoadType::MAIN_ROAD;
    if (s == "HIGHWAY" || s == "EXPRESSWAY") return RoadType::HIGHWAY;
    if (s == "HILL_ROAD" || s == "HILL") return RoadType::HILL_ROAD;
    return RoadType::UNKNOWN;
}

RoadEdge::RoadEdge()
    : fromNodeId(""), toNodeId(""), distanceKm(0.0),
      roadName(""), roadType(RoadType::UNKNOWN) {}

RoadEdge::RoadEdge(const std::string& from,
                   const std::string& to,
                   double distKm,
                   const std::string& name,
                   RoadType type)
    : fromNodeId(from), toNodeId(to), distanceKm(distKm),
      roadName(name), roadType(type) {}
