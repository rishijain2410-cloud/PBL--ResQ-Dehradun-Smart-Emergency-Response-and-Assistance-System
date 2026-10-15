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
        // option not found
        return estimate;
    }
    estimate.displayName = optionPtr->getDisplayName();
    if (!optionPtr->isSuitableFor(route.getTotalDistanceKm())) {
        estimate.note = optionPtr->unsuitableReason(route.getTotalDistanceKm());
        return estimate;
    }
    // suitable
    estimate.available = true;
    estimate.note = "";
    estimate.estimatedDurationMinutes = optionPtr->estimateDurationMinutes(route);
    estimate.estimatedCost = optionPtr->estimateCost(route);
    return estimate;
}

std::vector<TransportEstimate> TransportEstimator::estimateAll(const RouteResponse& route) const {
    std::vector<TransportEstimate> results;
    for (const auto& optPtr : repo.findAll()) {
        TransportEstimate e;
        e.mode = optPtr->getMode();
        e.displayName = optPtr->getDisplayName();
        e.distanceKm = route.getTotalDistanceKm();
        if (!optPtr->isSuitableFor(route.getTotalDistanceKm())) {
            e.available = false;
            e.note = optPtr->unsuitableReason(route.getTotalDistanceKm());
            e.estimatedDurationMinutes = 0.0;
            e.estimatedCost = 0.0;
        } else {
            e.available = true;
            e.note = "";
            e.estimatedDurationMinutes = optPtr->estimateDurationMinutes(route);
            e.estimatedCost = optPtr->estimateCost(route);
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
