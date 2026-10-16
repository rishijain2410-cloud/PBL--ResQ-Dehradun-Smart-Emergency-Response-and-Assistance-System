#include "model/AmbulanceOption.h"

AmbulanceOption::AmbulanceOption(const std::string& optionId,
                                 const std::string& displayName,
                                 double averageSpeedKmph,
                                 double costPerKm,
                                 double baseFare,
                                 double maxPracticalDistanceKm)
    : TransportOption(optionId, TransportMode::AMBULANCE, displayName,
                      averageSpeedKmph, costPerKm, baseFare, maxPracticalDistanceKm) {}

double AmbulanceOption::speedFor(RoadType /*roadType*/) const {
    return averageSpeedKmph;
}

double AmbulanceOption::fixedOverheadMinutes() const {
    return 0.0;
}

bool AmbulanceOption::isSuitableFor(double totalDistanceKm) const {
    return totalDistanceKm >= 0.0;
}

std::string AmbulanceOption::unsuitableReason(double /*totalDistanceKm*/) const {
    return "";
}