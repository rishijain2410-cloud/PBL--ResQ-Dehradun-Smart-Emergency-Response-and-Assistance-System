#pragma once

#include "repository/LocationRepository.h"
#include "dto/RouteRequest.h"
#include "dto/ApiResponse.h"

#include <string>

class RouteRequestValidator {
private:
    const ILocationRepository& repo;

    static std::string trim(const std::string& str);
    static bool isValidLocationId(const std::string& id);

public:
    explicit RouteRequestValidator(const ILocationRepository& repo);

    ApiResponse<RouteRequest> validate(const std::string& sourceId,
                                      const std::string& destinationId,
                                      const std::string& modeText) const;
};
