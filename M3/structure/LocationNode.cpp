#include "structure/LocationNode.h"

#include <iomanip>
#include <sstream>

using namespace std;

LocationNode::LocationNode()
    : nodeId(""),
      name(""),
      latitude(0.0),
      longitude(0.0) {}

LocationNode::LocationNode(const string& nodeId,
                           const string& name,
                           double latitude,
                           double longitude)
    : nodeId(nodeId),
      name(name),
      latitude(latitude),
      longitude(longitude) {}

LocationNode LocationNode::fromLocation(const Location& location) {
    return LocationNode(
        location.getLocationId(),
        location.getName(),
        location.getLatitude(),
        location.getLongitude()
    );
}

const string& LocationNode::getNodeId() const {
    return nodeId;
}

const string& LocationNode::getName() const {
    return name;
}

double LocationNode::getLatitude() const {
    return latitude;
}

double LocationNode::getLongitude() const {
    return longitude;
}

string LocationNode::toString() const {
    ostringstream ss;
    ss << "Node[" << nodeId << ": " << name
    << " @ (" << fixed << setprecision(4) << latitude << ", "
    << fixed << setprecision(4) << longitude << ")]";
    return ss.str();
}

bool LocationNode::operator==(const LocationNode& other) const {
    return nodeId == other.nodeId;
}

bool LocationNode::operator!=(const LocationNode& other) const {
    return !(*this == other);
}
