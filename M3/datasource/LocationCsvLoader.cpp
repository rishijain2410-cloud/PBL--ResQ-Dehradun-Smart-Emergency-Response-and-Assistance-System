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
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '\"') {
                token += '\"';
                ++i;
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

    for (auto& tok : tokens) {
        tok.erase(tok.begin(), find_if(tok.begin(), tok.end(), [](unsigned char c) {
            return !isspace(c);
        }));
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

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        string trimmed = line;
        trimmed.erase(trimmed.begin(), find_if(trimmed.begin(), trimmed.end(), [](unsigned char c) {
            return !isspace(c);
        }));

        if (trimmed.empty()) {
            continue;
        }

        if (trimmed[0] == '#') {
            continue;
        }

        vector<string> fields = parseCsvLine(line);

        if (!fields.empty()) {
            string firstColLower = fields[0];
            transform(firstColLower.begin(), firstColLower.end(), firstColLower.begin(),
                           [](unsigned char c) { return tolower(c); });
            if (firstColLower == "locationid" || firstColLower == "id") {
                continue;
            }
        }

        if (fields.size() < 5) {
            cerr << "[LocationCsvLoader Warning] Line " << lineNumber
                      << ": Insufficient columns (expected at least 5, got "
                      << fields.size() << "). Skipping row: '" << line << "'\n";
            continue;
        }

        try {
            const string& id = fields[0];
            const string& name = fields[1];

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

            LocationType type = Location::stringToLocationType(fields[4]);

            string address = (fields.size() >= 6) ? fields[5] : "";

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
