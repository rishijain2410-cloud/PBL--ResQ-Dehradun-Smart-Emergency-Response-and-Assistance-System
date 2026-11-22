#ifndef M3_SERVICE_NEARBY_SERVICES_FINDER_H
#define M3_SERVICE_NEARBY_SERVICES_FINDER_H

#include "../repository/ILocationRepository.h"
#include "../dto/ApiResponse.h"
#include <vector>
#include <utility>
#include <memory>
#include <string>

struct NearbyResult {
    double distanceKm;
    Location location;

    NearbyResult(double dist, const Location& loc) : distanceKm(dist), location(loc) {}

    std::string toJson() const;
};

class NearbyServicesFinder {
private:
    std::shared_ptr<ILocationRepository> repository;

public:
    explicit NearbyServicesFinder(std::shared_ptr<ILocationRepository> repo);

    ApiResponse<std::vector<NearbyResult>> findNearby(
        double latitude,
        double longitude,
        double radiusKm,
        LocationType typeFilter = LocationType::UNKNOWN);

    static std::string resultsToJson(const std::vector<NearbyResult>& results);
};

#endif // M3_SERVICE_NEARBY_SERVICES_FINDER_H
