#pragma once

#include "model/Location.h"

#include <vector>
#include <string>
#include <istream>

class LocationCsvLoader {
public:
    static std::vector<Location> loadFromFile(const std::string& filePath);

    static std::vector<Location> loadFromStream(std::istream& input);

private:
    static std::vector<std::string> parseCsvLine(const std::string& line);
};
