#pragma once

#include "model/Location.h"

#include <vector>
#include <string>
#include <istream>

class LocationCsvLoader {
public:
    /**
     * Reads a CSV file from disk and parses Location entities.
     * Skips comment rows (starting with '#') and table headers.
     * Malformed rows (invalid types, incorrect column counts, out-of-range coordinates)
     * are reported to std::cerr and gracefully skipped without terminating execution.
     *
     * @param filePath Absolute or relative path to CSV file.
     * @return std::vector<Location> containing all valid parsed locations.
     */
    static std::vector<Location> loadFromFile(const std::string& filePath);

    /**
     * Overload for parsing from an arbitrary std::istream.
     * Facilitates unit testing with in-memory string streams.
     *
     * @param input Stream supplying CSV text data.
     * @return std::vector<Location> containing all valid parsed locations.
     */
    static std::vector<Location> loadFromStream(std::istream& input);

private:
    /**
     * Splits a CSV line into tokens, supporting comma delimiters and quoted fields.
     */
    static std::vector<std::string> parseCsvLine(const std::string& line);
};
