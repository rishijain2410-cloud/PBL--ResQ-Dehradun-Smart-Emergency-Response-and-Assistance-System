#include "RoadNetworkCsvLoader.h"
#include "LocationCsvLoader.h"
#include <fstream>
#include <iostream>

std::vector<RoadEdge> RoadNetworkCsvLoader::loadFromFile(const std::string& filepath) {
    std::vector<RoadEdge> edges;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[RoadNetworkCsvLoader] Warning: Could not open file: " << filepath << std::endl;
        return edges;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (isHeader) {
            isHeader = false;
            continue;
        }

        std::vector<std::string> fields = LocationCsvLoader::parseCsvLine(line);
        if (fields.size() < 5) continue;

        try {
            std::string from = fields[0];
            std::string to = fields[1];
            double distKm = std::stod(fields[2]);
            std::string name = fields[3];
            RoadType type = stringToRoadType(fields[4]);

            edges.emplace_back(from, to, distKm, name, type);
        } catch (const std::exception& e) {
            std::cerr << "[RoadNetworkCsvLoader] Error parsing line: " << line << " (" << e.what() << ")" << std::endl;
        }
    }

    return edges;
}
