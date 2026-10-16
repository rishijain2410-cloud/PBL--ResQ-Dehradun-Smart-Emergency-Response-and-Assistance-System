#include "model/TransportOption.h"

using namespace std;

TransportOption::TransportOption(const string& optionId,
                                 TransportMode mode,
                                 const string& displayName,
                                 double averageSpeedKmph,
                                 double costPerKm,
                                 double baseFare,
                                 double maxPracticalDistanceKm)
    : optionId(optionId),
      mode(mode),
      displayName(displayName),
      averageSpeedKmph(averageSpeedKmph),
      costPerKm(costPerKm),
      baseFare(baseFare),
      maxPracticalDistanceKm(maxPracticalDistanceKm) {}

double TransportOption::estimateDurationMinutes(const RouteResponse& route) const {
    if (averageSpeedKmph <= 0.0) {
        return 0.0;
    }
    return route.getTotalDistanceKm() / averageSpeedKmph * 60.0;
}

double TransportOption::estimateCost(const RouteResponse& route) const {
    return baseFare + (costPerKm * route.getTotalDistanceKm());
}

const string& TransportOption::getOptionId() const {
    return optionId;
}

TransportMode TransportOption::getMode() const {
    return mode;
}

const string& TransportOption::getDisplayName() const {
    return displayName;
}

double TransportOption::getAverageSpeedKmph() const {
    return averageSpeedKmph;
}

double TransportOption::getCostPerKm() const {
    return costPerKm;
}

double TransportOption::getBaseFare() const {
    return baseFare;
}

double TransportOption::getMaxPracticalDistanceKm() const {
    return maxPracticalDistanceKm;
}
