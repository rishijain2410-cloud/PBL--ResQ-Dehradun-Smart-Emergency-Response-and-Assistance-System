#include "datasource/LocationCsvLoader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<std::string> LocationCsvLoader::parseCsvLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];

        if (ch == '\"') {
            // Check for escaped quote: ""
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '\"') {
                token += '\"';
                ++i; // Skip next quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            tokens.push_back(token);
            token.clear();
        } else {
            token += ch;
        }
    }
    tokens.push_back(token);

    // Trim outer whitespace for each token
    for (auto& tok : tokens) {
        // Trim leading spaces
        tok.erase(tok.begin(), std::find_if(tok.begin(), tok.end(), [](unsigned char c) {
            return !std::isspace(c);
        }));
        // Trim trailing spaces
        tok.erase(std::find_if(tok.rbegin(), tok.rend(), [](unsigned char c) {
            return !std::isspace(c);
        }).base(), tok.end());
    }

    return tokens;
}

std::vector<Location> LocationCsvLoader::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[LocationCsvLoader Error] Unable to open CSV file at path: "
                  << filePath << std::endl;
        return {};
    }
    return loadFromStream(file);
}

std::vector<Location> LocationCsvLoader::loadFromStream(std::istream& input) {
    std::vector<Location> locations;
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(input, line)) {
        lineNumber++;

        // Remove carriage return '\r' if line was saved with Windows CRLF
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Trim whitespace to check if empty
        std::string trimmed = line;
        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char c) {
            return !std::isspace(c);
        }));

        if (trimmed.empty()) {
            continue; // Skip blank lines
        }

        // Skip comments (e.g. lines starting with '#')
        if (trimmed[0] == '#') {
            continue;
        }

        std::vector<std::string> fields = parseCsvLine(line);

        // Check if this is the header row
        if (!fields.empty()) {
            std::string firstColLower = fields[0];
            std::transform(firstColLower.begin(), firstColLower.end(), firstColLower.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (firstColLower == "locationid" || firstColLower == "id") {
                continue; // Skip header row
            }
        }

        // Validate minimum column count: locationId, name, latitude, longitude, locationType
        if (fields.size() < 5) {
            std::cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                      << ": Insufficient columns (expected at least 5, got "
                      << fields.size() << "). Skipping row: '" << line << "'\n";
            continue;
        }

        try {
            const std::string& id = fields[0];
            const std::string& name = fields[1];
            
            // Parse coordinates
            double lat = 0.0;
            double lon = 0.0;
            try {
                lat = std::stod(fields[2]);
                lon = std::stod(fields[3]);
            } catch (const std::exception& e) {
                std::cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                          << ": Failed to parse coordinates ('" << fields[2] << "', '"
                          << fields[3] << "') as numbers: " << e.what()
                          << ". Skipping row.\n";
                continue;
            }

            // Parse location type
            LocationType type = Location::stringToLocationType(fields[4]);

            // Optional address column
            std::string address = (fields.size() >= 6) ? fields[5] : "";

            // Construct and validate Location object
            Location loc(id, name, lat, lon, type, address);
            locations.push_back(loc);

        } catch (const std::invalid_argument& ex) {
            std::cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                      << ": Validation failure: " << ex.what()
                      << ". Skipping row.\n";
        } catch (const std::exception& ex) {
            std::cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                      << ": Unexpected error: " << ex.what()
                      << ". Skipping row.\n";
        }
    }

    return locations;
}
