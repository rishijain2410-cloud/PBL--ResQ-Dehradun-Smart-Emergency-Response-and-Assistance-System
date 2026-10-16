#pragma once

#include "structure/RoadGraph.h"
#include "structure/RoadEdge.h"
#include "repository/LocationRepository.h"

#include <vector>

class RoadGraphBuilder {
public:
    static RoadGraph build(const ILocationRepository& repo, const std::vector<RoadEdge>& edges);
};
