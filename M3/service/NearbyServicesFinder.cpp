#include "service/NearbyServicesFinder.h"
#include "util/GeoUtil.h"

#include <algorithm>
#include <unordered_set>

using namespace std;

NearbyServicesFinder::NearbyServicesFinder(const ILocationRepository& repo)
    : repo(repo) {}

ApiResponse<vector<Location>> NearbyServicesFinder::findNearby(
    double latitude,
    double longitude,
    double radiusKm,
    const vector<LocationType>& filterTypes) const
{
    using ResultType = vector<Location>;

    if (latitude < -90.0 || latitude > 90.0 ||
        longitude < -180.0 || longitude > 180.0) {
        return ApiResponse<ResultType>::fail(
            "Invalid coordinates: latitude must be in [-90,90] and longitude in [-180,180].",
            ErrorCode::INVALID_ID);
    }

    if (radiusKm <= 0.0) {
        return ApiResponse<ResultType>::fail(
            "Invalid radius: radiusKm must be greater than 0.",
            ErrorCode::VALIDATION_ERROR);
    }

    vector<Location> all = repo.findAll();
    if (all.empty()) {
        return ApiResponse<ResultType>::fail(
            "No locations in repository.",
            ErrorCode::NO_RESULTS);
    }

    unordered_set<int> typeSet;
    for (const auto& t : filterTypes) {
        typeSet.insert(static_cast<int>(t));
    }
    bool filterAll = filterTypes.empty();

    vector<pair<double, Location>> candidates;
    candidates.reserve(all.size());

    for (const auto& loc : all) {
        if (!filterAll && typeSet.find(static_cast<int>(loc.getLocationType())) == typeSet.end()) {
            continue;
        }
        double dist = GeoUtil::haversineDistanceKm(
            latitude, longitude,
            loc.getLatitude(), loc.getLongitude());
        if (dist <= radiusKm) {
            candidates.emplace_back(dist, loc);
        }
    }

    if (candidates.empty()) {
        return ApiResponse<ResultType>::fail(
            "No locations found within the specified radius.",
            ErrorCode::NO_RESULTS);
    }

    sort(candidates.begin(), candidates.end(),
              [](const pair<double, Location>& a,
                 const pair<double, Location>& b) {
                  return a.first < b.first;
              });

    ResultType result;
    result.reserve(candidates.size());
    for (const auto& p : candidates) {
        result.push_back(p.second);
    }

    return ApiResponse<ResultType>::ok(result,
        "Found " + to_string(result.size()) + " location(s) within " +
        to_string(radiusKm) + " km.");
}
