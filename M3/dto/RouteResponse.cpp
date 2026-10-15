#include "dto/RouteResponse.h"

#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

RouteResponse::RouteResponse()
    : startNodeId(""),
      endNodeId(""),
      totalDistanceKm(0.0),
      pathNodeIds({}),
      segments({}),
      nodesExplored(0) {}

RouteResponse::RouteResponse(const string& startNodeId,
                             const string& endNodeId,
                             double totalDistanceKm,
                             const vector<string>& pathNodeIds,
                             const vector<RouteSegment>& segments,
                             size_t nodesExplored)
    : startNodeId(startNodeId),
      endNodeId(endNodeId),
      totalDistanceKm(totalDistanceKm),
      pathNodeIds(pathNodeIds),
      segments(segments),
      nodesExplored(nodesExplored) {

    double sumDist = 0.0;
    for (const auto& seg : segments) {
        sumDist += seg.getDistanceKm();
    }

    if (segments.empty()) {
        if (abs(totalDistanceKm) > 1e-6) {
            throw invalid_argument(
                "Invariant Error: Empty route segments must have totalDistanceKm == 0.0"
            );
        }
    } else {
        if (abs(totalDistanceKm - sumDist) > 1e-4) {
            ostringstream ss;
            ss << "Invariant Error: totalDistanceKm (" << totalDistanceKm
               << ") does not equal sum of segment distances (" << sumDist << ").";
            throw invalid_argument(ss.str());
        }
    }
}

const string& RouteResponse::getStartNodeId() const {
    return startNodeId;
}

const string& RouteResponse::getEndNodeId() const {
    return endNodeId;
}

double RouteResponse::getTotalDistanceKm() const {
    return totalDistanceKm;
}

const vector<string>& RouteResponse::getPathNodeIds() const {
    return pathNodeIds;
}

const vector<RouteSegment>& RouteResponse::getSegments() const {
    return segments;
}

size_t RouteResponse::getNodesExplored() const {
    return nodesExplored;
}

size_t RouteResponse::segmentCount() const {
    return segments.size();
}

string RouteResponse::toString() const {
    ostringstream ss;
    if (pathNodeIds.empty()) {
        return "Route[Empty]";
    }

    for (size_t i = 0; i < pathNodeIds.size(); ++i) {
        ss << pathNodeIds[i];
        if (i + 1 < pathNodeIds.size()) {
            ss << " -> ";
        }
    }

    ss << " (Total: " << fixed << setprecision(2) << totalDistanceKm
       << " km across " << segments.size() << " segment" << (segments.size() == 1 ? "" : "s")
       << ", " << nodesExplored << " nodes explored)";
    return ss.str();
}
