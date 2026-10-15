#pragma once

#include "repository/TransportOptionRepository.h"
#include "dto/TransportEstimate.h"
#include "dto/RouteResponse.h"
#include "model/TransportMode.h"
#include <vector>

class TransportEstimator {
public:
    explicit TransportEstimator(const ITransportOptionRepository& repo);

    TransportEstimate estimateFor(TransportMode mode, const RouteResponse& route) const;

    std::vector<TransportEstimate> estimateAll(const RouteResponse& route) const;

private:
    const ITransportOptionRepository& repo;
};
