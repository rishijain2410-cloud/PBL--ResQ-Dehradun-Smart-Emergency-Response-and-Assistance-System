#include "service/TransportEstimator.h"
#include "datasource/TransportOptionCsvLoader.h"
#include "repository/TransportOptionRepository.h"
#include "dto/TransportEstimate.h"
#include "dto/RouteResponse.h"
#include "model/TransportMode.h"
#include <algorithm>
#include <vector>

TransportEstimator::TransportEstimator(const ITransportOptionRepository& repo) : repo(repo) {}

TransportEstimate TransportEstimator::estimateFor(TransportMode mode, const RouteResponse& route) const {
    auto optionPtr = repo.findByMode(mode);
    TransportEstimate estimate;
    estimate.mode = mode;
    estimate.available = false;
    estimate.note = "Transport mode not available";
    estimate.distanceKm = route.getTotalDistanceKm();
    estimate.estimatedDurationMinutes = 0.0;
    estimate.estimatedCost = 0.0;
    if (!optionPtr) {
        return estimate;
    }
    estimate.displayName = optionPtr->getDisplayName();
    const double distanceKm = route.getTotalDistanceKm();
    if (distanceKm > optionPtr->getMaxPracticalDistanceKm()) {
        estimate.note = "Distance exceeds the practical range for this mode";
        return estimate;
    }
    if (distanceKm < 0.0 || optionPtr->getAverageSpeedKmph() <= 0.0) {
        estimate.note = optionPtr->unsuitableReason(distanceKm);
        return estimate;
    }
    // The ambulance estimate is the main one for emergency routing.
    estimate.available = true;
    estimate.note = "";
    estimate.estimatedDurationMinutes = distanceKm / optionPtr->getAverageSpeedKmph() * 60.0;
    estimate.estimatedCost = optionPtr->getBaseFare() + optionPtr->getCostPerKm() * distanceKm;
    return estimate;
}

std::vector<TransportEstimate> TransportEstimator::estimateAll(const RouteResponse& route) const {
    std::vector<TransportEstimate> results;
    for (const auto& optPtr : repo.findAll()) {
        TransportEstimate e;
        e.mode = optPtr->getMode();
        e.displayName = optPtr->getDisplayName();
        e.distanceKm = route.getTotalDistanceKm();
        const double distanceKm = route.getTotalDistanceKm();
        if (distanceKm > optPtr->getMaxPracticalDistanceKm()) {
            e.available = false;
            e.note = "Distance exceeds the practical range for this mode";
            e.estimatedDurationMinutes = 0.0;
            e.estimatedCost = 0.0;
        } else if (distanceKm < 0.0 || optPtr->getAverageSpeedKmph() <= 0.0) {
            e.available = false;
            e.note = optPtr->unsuitableReason(distanceKm);
            e.estimatedDurationMinutes = 0.0;
            e.estimatedCost = 0.0;
        } else {
            e.available = true;
            e.note = "";
            e.estimatedDurationMinutes = distanceKm / optPtr->getAverageSpeedKmph() * 60.0;
            e.estimatedCost = optPtr->getBaseFare() + optPtr->getCostPerKm() * distanceKm;
        }
        results.push_back(e);
    }
    // Stable sort: available first, then by duration ascending
    std::stable_sort(results.begin(), results.end(), [](const TransportEstimate& a, const TransportEstimate& b) {
        if (a.available != b.available) return a.available > b.available; // true first
        if (a.available && b.available) return a.estimatedDurationMinutes < b.estimatedDurationMinutes;
        return false;
    });
    return results;
}
