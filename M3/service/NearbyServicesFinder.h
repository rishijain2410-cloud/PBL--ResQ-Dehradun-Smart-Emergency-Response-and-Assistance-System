#pragma once

#include "repository/LocationRepository.h"
#include "model/Location.h"
#include "dto/ApiResponse.h"

#include <vector>

class NearbyServicesFinder {
private:
    const ILocationRepository& repo;

public:
    explicit NearbyServicesFinder(const ILocationRepository& repo);

    ApiResponse<std::vector<Location>> findNearby(
        double latitude,
        double longitude,
        double radiusKm,
        const std::vector<LocationType>& filterTypes) const;
};
