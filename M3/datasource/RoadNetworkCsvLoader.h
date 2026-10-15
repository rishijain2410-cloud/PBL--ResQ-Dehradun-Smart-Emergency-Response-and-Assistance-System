#pragma once

#include "structure/RoadEdge.h"

#include <vector>
#include <string>
#include <istream>

class RoadNetworkCsvLoader {
public:
    static std::vector<RoadEdge> loadFromFile(const std::string& filePath);

    static std::vector<RoadEdge> loadFromStream(std::istream& input);

private:
    static std::vector<std::string> parseCsvLine(const std::string& line);
};
