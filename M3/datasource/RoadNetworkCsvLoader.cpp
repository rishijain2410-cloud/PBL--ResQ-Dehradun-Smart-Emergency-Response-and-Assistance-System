#include "datasource/RoadNetworkCsvLoader.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

using namespace std;

vector<string> RoadNetworkCsvLoader::parseCsvLine(const string& line) {
    vector<string> tokens;
    string token;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];

        if (ch == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                token += '"';
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

vector<RoadEdge> RoadNetworkCsvLoader::loadFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "[RoadNetworkCsvLoader Error] Unable to open CSV file at path: "
                  << filePath << endl;
        return {};
    }
    return loadFromStream(file);
}

vector<RoadEdge> RoadNetworkCsvLoader::loadFromStream(istream& input) {
    vector<RoadEdge> edges;
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

        vector<string> fields = parseCsvLine(line);

        if (!fields.empty()) {
            string firstColLower = fields[0];
            transform(firstColLower.begin(), firstColLower.end(), firstColLower.begin(),
                           [](unsigned char c) { return tolower(c); });
            if (firstColLower == "edgeid" || firstColLower == "id") {
                continue;
            }
        }

        if (fields.size() < 6) {
            cerr << "[RoadNetworkCsvLoader Warning] Line " << lineNumber
                      << ": Insufficient columns (expected 6, got " << fields.size()
                      << "). Skipping row: '" << line << "'\n";
            continue;
        }

        try {
            const string& edgeId = fields[0];
            const string& fromId = fields[1];
            const string& toId = fields[2];
            const string& roadName = fields[3];

            double distKm = 0.0;
            try {
                distKm = stod(fields[4]);
            } catch (const exception& e) {
                cerr << "[RoadNetworkCsvLoader Warning] Line " << lineNumber
                          << ": Could not parse distanceKm '" << fields[4]
                          << "' as a number: " << e.what() << ". Skipping row.\n";
                continue;
            }

            RoadType type = RoadEdge::stringToRoadType(fields[5]);

            RoadEdge edge(edgeId, fromId, toId, roadName, distKm, type);
            edges.push_back(edge);

        } catch (const invalid_argument& ex) {
            cerr << "[RoadNetworkCsvLoader Warning] Line " << lineNumber
                      << ": Validation failure: " << ex.what()
                      << ". Skipping row.\n";
        } catch (const exception& ex) {
            cerr << "[RoadNetworkCsvLoader Warning] Line " << lineNumber
                      << ": Unexpected error: " << ex.what()
                      << ". Skipping row.\n";
        }
    }

    return edges;
}
