#include "TransportEstimator.h"

TransportEstimator::TransportEstimator() {
    options.push_back(std::make_shared<AmbulanceOption>());
    options.push_back(std::make_shared<TwoWheelerOption>());
    options.push_back(std::make_shared<PublicTransitOption>());
    options.push_back(std::make_shared<WalkingOption>());
}

void TransportEstimator::estimateAll(RouteResponse& route) const {
    const auto& segments = route.getSegments();
    double totalDist = route.getTotalDistanceKm();

    for (const auto& opt : options) {
        if (opt->isSuitable(totalDist)) {
            double minutes = opt->calculateTravelTimeMinutes(segments);
            double cost = opt->calculateCost(totalDist);
            route.addTravelEstimate(opt->getModeName(), minutes, cost);
        } else {
            // Unsuitable for extreme distances (e.g. walking > 5 km)
            route.addTravelEstimate(opt->getModeName() + " (Not Recommended)", -1.0, 0.0);
        }
    }
}
