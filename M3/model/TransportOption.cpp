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
    const auto& segments = route.getSegments();
    if (segments.empty()) {
        return 0.0;
    }

    double totalMinutes = 0.0;
    for (const auto& seg : segments) {
        double spd = speedFor(seg.getRoadType());
        if (spd <= 0.0) {
            spd = (averageSpeedKmph > 0.0 ? averageSpeedKmph : 10.0);
        }
        totalMinutes += (seg.getDistanceKm() / spd) * 60.0;
    }

    totalMinutes += fixedOverheadMinutes();
    return totalMinutes;
}

double TransportOption::estimateCost(const RouteResponse& route) const {
    if (route.getTotalDistanceKm() <= 0.0) {
        return 0.0;
    }
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
