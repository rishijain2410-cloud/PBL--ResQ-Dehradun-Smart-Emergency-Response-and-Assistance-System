#pragma once

#include "model/TransportMode.h"
#include <string>

struct TransportEstimate {
    TransportMode mode;
    std::string displayName;
    bool available;
    std::string note;
    double distanceKm;
    double estimatedDurationMinutes;
    double estimatedCost;
};
