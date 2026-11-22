#ifndef M3_STRUCTURE_LOCATION_NODE_H
#define M3_STRUCTURE_LOCATION_NODE_H

#include "../model/Location.h"
#include <string>

class LocationNode {
private:
    Location location;

public:
    LocationNode();
    explicit LocationNode(const Location& loc);

    const std::string& getId() const { return location.getId(); }
    const std::string& getName() const { return location.getName(); }
    double getLatitude() const { return location.getLatitude(); }
    double getLongitude() const { return location.getLongitude(); }
    LocationType getType() const { return location.getType(); }
    const Location& getLocation() const { return location; }
    Location& getLocationRef() { return location; }
};

#endif // M3_STRUCTURE_LOCATION_NODE_H
