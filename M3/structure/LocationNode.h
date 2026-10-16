#pragma once

#include "model/Location.h"
#include <string>

class LocationNode {
private:
    std::string nodeId;
    std::string name;
    double latitude;
    double longitude;

public:
    LocationNode();

    LocationNode(const std::string& nodeId,
                 const std::string& name,
                 double latitude,
                 double longitude);

    static LocationNode fromLocation(const Location& location);

    const std::string& getNodeId() const;
    const std::string& getName() const;
    double getLatitude() const;
    double getLongitude() const;

    std::string toString() const;

    bool operator==(const LocationNode& other) const;
    bool operator!=(const LocationNode& other) const;
};
