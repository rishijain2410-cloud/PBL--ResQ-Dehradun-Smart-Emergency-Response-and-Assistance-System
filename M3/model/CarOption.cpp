#include "model/CarOption.h"

using namespace std;

CarOption::CarOption(const string& optionId,
                     const string& displayName,
                     double averageSpeedKmph,
                     double costPerKm,
                     double baseFare,
                     double maxPracticalDistanceKm)
    : TransportOption(optionId,
                      TransportMode::CAR,
                      displayName,
                      averageSpeedKmph,
                      costPerKm,
                      baseFare,
                      maxPracticalDistanceKm) {}

double CarOption::speedFor(RoadType roadType) const {
    switch (roadType) {
        case RoadType::HIGHWAY:     return DEMO_HIGHWAY_SPEED_KMPH;
        case RoadType::MAIN_ROAD:   return DEMO_MAIN_ROAD_SPEED_KMPH;
        case RoadType::CITY_STREET: return DEMO_CITY_STREET_SPEED_KMPH;
        case RoadType::HILL_ROAD:   return DEMO_HILL_ROAD_SPEED_KMPH;
        default:                    return averageSpeedKmph;
    }
}

double CarOption::fixedOverheadMinutes() const {
    return DEMO_OVERHEAD_MINUTES;
}

bool CarOption::isSuitableFor(double totalDistanceKm) const {
    return totalDistanceKm >= 0.0;
}

string CarOption::unsuitableReason(double /*totalDistanceKm*/) const {
    return "";
}
