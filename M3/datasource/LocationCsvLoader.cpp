#include "LocationCsvLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

std::string LocationCsvLoader::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> LocationCsvLoader::parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    fields.push_back(trim(current));
    return fields;
}

std::vector<Location> LocationCsvLoader::loadFromFile(const std::string& filepath) {
    std::vector<Location> locations;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[LocationCsvLoader] Warning: Could not open file: " << filepath << std::endl;
        return locations;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (isHeader) {
            isHeader = false;
            continue;
        }

        std::vector<std::string> fields = parseCsvLine(line);
        if (fields.size() < 5) continue;

        try {
            std::string id = fields[0];
            std::string name = fields[1];
            double lat = std::stod(fields[2]);
            double lon = std::stod(fields[3]);
            LocationType type = stringToLocationType(fields[4]);
            std::string addr = (fields.size() >= 6) ? fields[5] : "";

            locations.emplace_back(id, name, lat, lon, type, addr);
        } catch (const std::exception& e) {
            std::cerr << "[LocationCsvLoader] Error parsing line: " << line << " (" << e.what() << ")" << std::endl;
        }
    }

    return locations;
}
