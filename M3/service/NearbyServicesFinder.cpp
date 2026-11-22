#include "NearbyServicesFinder.h"
#include "../util/GeoUtil.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

std::string NearbyResult::toJson() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{"
       << "\"distanceKm\":" << distanceKm << ","
       << "\"location\":" << location.toJson()
       << "}";
    return ss.str();
}

NearbyServicesFinder::NearbyServicesFinder(std::shared_ptr<ILocationRepository> repo)
    : repository(repo) {}

ApiResponse<std::vector<NearbyResult>> NearbyServicesFinder::findNearby(
    double latitude,
    double longitude,
    double radiusKm,
    LocationType typeFilter) {

    // 1. Boundary & validation checks (satisfying Demo d)
    if (latitude < -90.0 || latitude > 90.0) {
        return ApiResponse<std::vector<NearbyResult>>::error(
            "VALIDATION_ERROR", "Latitude must be between -90.0 and 90.0");
    }
    if (longitude < -180.0 || longitude > 180.0) {
        return ApiResponse<std::vector<NearbyResult>>::error(
            "VALIDATION_ERROR", "Longitude must be between -180.0 and 180.0");
    }
    if (radiusKm <= 0.0) {
        return ApiResponse<std::vector<NearbyResult>>::error(
            "VALIDATION_ERROR", "Search radius must be strictly positive (greater than 0 km)");
    }

    if (!repository) {
        return ApiResponse<std::vector<NearbyResult>>::error(
            "INTERNAL_ERROR", "Location repository is null");
    }

    // 2. Fetch candidates from repository
    std::vector<Location> allLocations = (typeFilter == LocationType::UNKNOWN)
        ? repository->findAll()
        : repository->findByType(typeFilter);

    std::vector<NearbyResult> matches;

    // 3. Compute Haversine distance and filter
    for (const auto& loc : allLocations) {
        double dist = GeoUtil::haversineDistanceKm(
            latitude, longitude, loc.getLatitude(), loc.getLongitude());
        if (dist <= radiusKm) {
            matches.emplace_back(dist, loc);
        }
    }

    // 4. Remote coordinate check producing clean NO_RESULTS (satisfying Demo e)
    if (matches.empty()) {
        return ApiResponse<std::vector<NearbyResult>>::error(
            "NO_RESULTS", "No facilities found within the specified search radius");
    }

    // 5. Sort matching candidates in ascending order by distance (O(N log N))
    std::sort(matches.begin(), matches.end(), [](const NearbyResult& a, const NearbyResult& b) {
        return a.distanceKm < b.distanceKm;
    });

    return ApiResponse<std::vector<NearbyResult>>::ok(
        matches, "Found " + std::to_string(matches.size()) + " facilities nearby");
}

std::string NearbyServicesFinder::resultsToJson(const std::vector<NearbyResult>& results) {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < results.size(); ++i) {
        ss << results[i].toJson();
        if (i + 1 < results.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}
