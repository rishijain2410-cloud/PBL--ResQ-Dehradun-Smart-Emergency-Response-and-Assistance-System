#include "structure/RoadEdge.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

using namespace std;

RoadEdge::RoadEdge()
    : edgeId(""),
      fromNodeId(""),
      toNodeId(""),
      roadName(""),
      distanceKm(0.0),
      roadType(RoadType::CITY_STREET) {}

RoadEdge::RoadEdge(const string& edgeId,
                   const string& fromNodeId,
                   const string& toNodeId,
                   const string& roadName,
                   double distanceKm,
                   RoadType roadType)
    : edgeId(edgeId),
      fromNodeId(fromNodeId),
      toNodeId(toNodeId),
      roadName(roadName),
      distanceKm(distanceKm),
      roadType(roadType) {

    // 1. Validate ID strings
    if (edgeId.empty()) {
        throw invalid_argument("Validation Error: edgeId cannot be empty.");
    }
    if (fromNodeId.empty()) {
        throw invalid_argument("Validation Error: fromNodeId cannot be empty.");
    }
    if (toNodeId.empty()) {
        throw invalid_argument("Validation Error: toNodeId cannot be empty.");
    }

    // 2. Validate self-loop prevention
    if (fromNodeId == toNodeId) {
        throw invalid_argument(
            "Validation Error: Self-loop edges are not permitted. fromNodeId and toNodeId cannot be identical ('" +
            fromNodeId + "')."
        );
    }

    // 3. Validate road name
    if (roadName.empty()) {
        throw invalid_argument("Validation Error: roadName cannot be empty.");
    }

    // 4. Validate non-negative weight (Dijkstra's shortest path requirement)
    // Shortest path algorithms like Dijkstra assume non-negative edge weights to guarantee correctness.
    if (distanceKm <= 0.0) {
        ostringstream ss;
        ss << "Validation Error: distanceKm must be strictly positive (> 0.0) for Dijkstra compatibility. "
           << "Received: " << distanceKm;
        throw invalid_argument(ss.str());
    }
}

const string& RoadEdge::getEdgeId() const {
    return edgeId;
}

const string& RoadEdge::getFromNodeId() const {
    return fromNodeId;
}

const string& RoadEdge::getToNodeId() const {
    return toNodeId;
}

const string& RoadEdge::getRoadName() const {
    return roadName;
}

double RoadEdge::getDistanceKm() const {
    return distanceKm;
}

RoadType RoadEdge::getRoadType() const {
    return roadType;
}

string RoadEdge::toString() const {
    ostringstream ss;
    ss << "Edge[" << edgeId << ": " << fromNodeId << " -> " << toNodeId
       << " via '" << roadName << "' (" << roadTypeToString(roadType) << "), "
    << fixed << setprecision(2) << distanceKm << " km]";
    return ss.str();
}

string RoadEdge::roadTypeToString(RoadType type) {
    switch (type) {
        case RoadType::HIGHWAY:     return "HIGHWAY";
        case RoadType::MAIN_ROAD:   return "MAIN_ROAD";
        case RoadType::CITY_STREET: return "CITY_STREET";
        case RoadType::HILL_ROAD:   return "HILL_ROAD";
        default:                    return "UNKNOWN";
    }
}

RoadType RoadEdge::stringToRoadType(const string& typeStr) {
    string s = typeStr;
    // Trim leading whitespace
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    // Trim trailing whitespace
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());

    string upperStr;
    upperStr.reserve(s.size());
    for (char c : s) {
        upperStr.push_back(static_cast<char>(toupper(static_cast<unsigned char>(c))));
    }

    if (upperStr == "HIGHWAY")     return RoadType::HIGHWAY;
    if (upperStr == "MAIN_ROAD")   return RoadType::MAIN_ROAD;
    if (upperStr == "CITY_STREET") return RoadType::CITY_STREET;
    if (upperStr == "HILL_ROAD")   return RoadType::HILL_ROAD;

    throw invalid_argument(
        "Validation Error: Unknown RoadType string '" + typeStr +
        "'. Supported types: HIGHWAY, MAIN_ROAD, CITY_STREET, HILL_ROAD."
    );
}

bool RoadEdge::operator==(const RoadEdge& other) const {
    return edgeId == other.edgeId &&
           fromNodeId == other.fromNodeId &&
           toNodeId == other.toNodeId;
}

bool RoadEdge::operator!=(const RoadEdge& other) const {
    return !(*this == other);
}
