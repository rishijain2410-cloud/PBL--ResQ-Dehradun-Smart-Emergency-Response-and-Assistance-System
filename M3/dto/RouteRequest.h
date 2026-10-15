#pragma once

#include "model/TransportMode.h"
#include <string>

class RouteRequest {
private:
    std::string sourceLocationId;
    std::string destinationLocationId;
    TransportMode mode;

public:
    RouteRequest();

    RouteRequest(const std::string& sourceLocationId,
                 const std::string& destinationLocationId,
                 TransportMode mode);

    const std::string& getSourceLocationId() const;
    const std::string& getDestinationLocationId() const;
    TransportMode getMode() const;

    std::string toString() const;

    bool operator==(const RouteRequest& other) const;
    bool operator!=(const RouteRequest& other) const;
};
