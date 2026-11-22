#ifndef M3_SERVICE_ROAD_GRAPH_BUILDER_H
#define M3_SERVICE_ROAD_GRAPH_BUILDER_H

#include "../structure/RoadGraph.h"
#include "../repository/ILocationRepository.h"
#include "../structure/RoadEdge.h"
#include <vector>
#include <memory>

class RoadGraphBuilder {
public:
    static std::shared_ptr<RoadGraph> buildGraph(
        const ILocationRepository& locationRepo,
        const std::vector<RoadEdge>& edges,
        bool bidirectional = true);
};

#endif // M3_SERVICE_ROAD_GRAPH_BUILDER_H
