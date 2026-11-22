#ifndef M3_DATASOURCE_ROAD_NETWORK_CSV_LOADER_H
#define M3_DATASOURCE_ROAD_NETWORK_CSV_LOADER_H

#include "../structure/RoadEdge.h"
#include <vector>
#include <string>

class RoadNetworkCsvLoader {
public:
    static std::vector<RoadEdge> loadFromFile(const std::string& filepath);
};

#endif // M3_DATASOURCE_ROAD_NETWORK_CSV_LOADER_H
