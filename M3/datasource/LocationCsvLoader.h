#ifndef M3_DATASOURCE_LOCATION_CSV_LOADER_H
#define M3_DATASOURCE_LOCATION_CSV_LOADER_H

#include "../model/Location.h"
#include <vector>
#include <string>

class LocationCsvLoader {
public:
    static std::vector<Location> loadFromFile(const std::string& filepath);
    static std::vector<std::string> parseCsvLine(const std::string& line);
    static std::string trim(const std::string& str);
};

#endif // M3_DATASOURCE_LOCATION_CSV_LOADER_H
