#include "dto/RouteSegment.h"

#include <sstream>
#include <iomanip>

using namespace std;

RouteSegment::RouteSegment()
    : fromNodeId(""),
      toNodeId(""),
      roadName(""),
      distanceKm(0.0),
      roadType(RoadType::CITY_STREET) {}

RouteSegment::RouteSegment(const string& fromNodeId,
                           const string& toNodeId,
                           const string& roadName,
                           double distanceKm,
                           RoadType roadType)
    : fromNodeId(fromNodeId),
      toNodeId(toNodeId),
      roadName(roadName),
      distanceKm(distanceKm),
      roadType(roadType) {}

const string& RouteSegment::getFromNodeId() const {
    return fromNodeId;
}

const string& RouteSegment::getToNodeId() const {
    return toNodeId;
}

const string& RouteSegment::getRoadName() const {
    return roadName;
}

double RouteSegment::getDistanceKm() const {
    return distanceKm;
}

RoadType RouteSegment::getRoadType() const {
    return roadType;
}

string RouteSegment::toString() const {
    ostringstream ss;
    ss << "[" << fromNodeId << " -> " << toNodeId
       << " via '" << roadName << "' ("
       << RoadEdge::roadTypeToString(roadType) << "): "
       << fixed << setprecision(2) << distanceKm << " km]";
    return ss.str();
}

bool RouteSegment::operator==(const RouteSegment& other) const {
    return fromNodeId == other.fromNodeId &&
           toNodeId == other.toNodeId &&
           roadName == other.roadName &&
           roadType == other.roadType &&
        std::abs(distanceKm - other.distanceKm) < 1e-6;
}

bool RouteSegment::operator!=(const RouteSegment& other) const {
    return !(*this == other);
}
