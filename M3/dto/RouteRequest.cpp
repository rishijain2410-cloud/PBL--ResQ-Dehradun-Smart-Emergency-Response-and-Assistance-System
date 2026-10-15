#include "dto/RouteRequest.h"

#include <sstream>

using namespace std;

RouteRequest::RouteRequest()
    : sourceLocationId(""),
      destinationLocationId(""),
      mode(TransportMode::CAR) {}

RouteRequest::RouteRequest(const string& sourceLocationId,
                           const string& destinationLocationId,
                           TransportMode mode)
    : sourceLocationId(sourceLocationId),
      destinationLocationId(destinationLocationId),
      mode(mode) {}

const string& RouteRequest::getSourceLocationId() const {
    return sourceLocationId;
}

const string& RouteRequest::getDestinationLocationId() const {
    return destinationLocationId;
}

TransportMode RouteRequest::getMode() const {
    return mode;
}

string RouteRequest::toString() const {
    ostringstream ss;
    ss << "RouteRequest[From: " << sourceLocationId
       << " -> To: " << destinationLocationId
       << ", Mode: " << ::toString(mode) << "]";
    return ss.str();
}

bool RouteRequest::operator==(const RouteRequest& other) const {
    return sourceLocationId == other.sourceLocationId &&
           destinationLocationId == other.destinationLocationId &&
           mode == other.mode;
}

bool RouteRequest::operator!=(const RouteRequest& other) const {
    return !(*this == other);
}
