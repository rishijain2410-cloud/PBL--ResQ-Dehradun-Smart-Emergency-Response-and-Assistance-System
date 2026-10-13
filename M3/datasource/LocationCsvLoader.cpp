#include "datasource/LocationCsvLoader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

vector<string> LocationCsvLoader::parseCsvLine(const string& line) {
    vector<string> tokens;
    string token;
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
        tok.erase(tok.begin(), find_if(tok.begin(), tok.end(), [](unsigned char c) {
            return !isspace(c);
        }));
        // Trim trailing spaces
        tok.erase(find_if(tok.rbegin(), tok.rend(), [](unsigned char c) {
            return !isspace(c);
        }).base(), tok.end());
    }

    return tokens;
}

vector<Location> LocationCsvLoader::loadFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "[LocationCsvLoader Error] Unable to open CSV file at path: "
             << filePath << endl;
        return {};
    }
    return loadFromStream(file);
}

vector<Location> LocationCsvLoader::loadFromStream(istream& input) {
    vector<Location> locations;
    string line;
    size_t lineNumber = 0;

    while (getline(input, line)) {
        lineNumber++;

        // Remove carriage return '\r' if line was saved with Windows CRLF
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Trim whitespace to check if empty
        string trimmed = line;
        trimmed.erase(trimmed.begin(), find_if(trimmed.begin(), trimmed.end(), [](unsigned char c) {
            return !isspace(c);
        }));

        if (trimmed.empty()) {
            continue; // Skip blank lines
        }

        // Skip comments (e.g. lines starting with '#')
        if (trimmed[0] == '#') {
            continue;
        }

        vector<string> fields = parseCsvLine(line);

        // Check if this is the header row
        if (!fields.empty()) {
            string firstColLower = fields[0];
            transform(firstColLower.begin(), firstColLower.end(), firstColLower.begin(),
                      [](unsigned char c) { return tolower(c); });
            if (firstColLower == "locationid" || firstColLower == "id") {
                continue; // Skip header row
            }
        }

        // Validate minimum column count: locationId, name, latitude, longitude, locationType
        if (fields.size() < 5) {
            cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                      << ": Insufficient columns (expected at least 5, got "
                      << fields.size() << "). Skipping row: '" << line << "'\n";
            continue;
        }

        try {
            const string& id = fields[0];
            const string& name = fields[1];
            
            // Parse coordinates
            double lat = 0.0;
            double lon = 0.0;
            try {
                lat = stod(fields[2]);
                lon = stod(fields[3]);
            } catch (const exception& e) {
                cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                          << ": Failed to parse coordinates ('" << fields[2] << "', '"
                          << fields[3] << "') as numbers: " << e.what()
                          << ". Skipping row.\n";
                continue;
            }

            // Parse location type
            LocationType type = Location::stringToLocationType(fields[4]);

            // Optional address column
            string address = (fields.size() >= 6) ? fields[5] : "";

            // Construct and validate Location object
            Location loc(id, name, lat, lon, type, address);
            locations.push_back(loc);

        } catch (const invalid_argument& ex) {
            cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                      << ": Validation failure: " << ex.what()
                      << ". Skipping row.\n";
        } catch (const exception& ex) {
            cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                      << ": Unexpected error: " << ex.what()
                      << ". Skipping row.\n";
        }
    }

    return locations;
}
