#include "datasource/TransportOptionCsvLoader.h"
#include "model/AmbulanceOption.h"
#include "model/CarOption.h"
#include "model/TransportMode.h"
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <algorithm>

using namespace std;

static vector<string> splitCsv(const string& line) {
    vector<string> tokens;
    string token;
    bool inQuotes = false;
    for (char ch : line) {
        if (ch == '"') {
            inQuotes = !inQuotes;
        } else if (ch == ',' && !inQuotes) {
            tokens.push_back(token);
            token.clear();
        } else {
            token += ch;
        }
    }
    tokens.push_back(token);
    for (auto& t : tokens) {
        size_t start = t.find_first_not_of(" \t\n\r");
        size_t end = t.find_last_not_of(" \t\n\r");
        if (start == string::npos) t.clear(); else t = t.substr(start, end - start + 1);
    }
    return tokens;
}

shared_ptr<TransportOption> TransportOptionCsvLoader::createTransportOption(
        const string& optionId,
        TransportMode mode,
        const string& displayName,
        double averageSpeedKmph,
        double costPerKm,
        double baseFare,
        double maxPracticalDistanceKm) {
    switch (mode) {
        case TransportMode::AMBULANCE:
            return make_shared<AmbulanceOption>(optionId, displayName, averageSpeedKmph, costPerKm, baseFare, maxPracticalDistanceKm);
        case TransportMode::CAR:
            return make_shared<CarOption>(optionId, displayName, averageSpeedKmph, costPerKm, baseFare, maxPracticalDistanceKm);
    }
    return nullptr;
}

vector<shared_ptr<TransportOption>> TransportOptionCsvLoader::loadFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "[TransportOptionCsvLoader Error] Unable to open CSV file: " << filePath << endl;
        return {};
    }
    return loadFromStream(file);
}

vector<shared_ptr<TransportOption>> TransportOptionCsvLoader::loadFromStream(istream& input) {
    vector<shared_ptr<TransportOption>> options;
    string line; size_t lineNumber = 0; unordered_set<string> seenIds;
    while (getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        string trimmed = line;
        trimmed.erase(trimmed.begin(), find_if(trimmed.begin(), trimmed.end(), [](unsigned char c){ return !isspace(c); }));
        trimmed.erase(find_if(trimmed.rbegin(), trimmed.rend(), [](unsigned char c){ return !isspace(c); }).base(), trimmed.end());
        if (trimmed.empty() || trimmed[0] == '#') continue;
        auto fields = splitCsv(line);
        if (!fields.empty() && fields[0] == "optionId") { continue; }
        if (fields.size() < 7) {
            cerr << "[TransportOptionCsvLoader Warning] Line " << lineNumber << ": Insufficient columns (expected 7) - Skipping row: '" << line << "'" << endl;
            continue;
        }
        const string& id = fields[0];
        const string& modeStr = fields[1];
        const string& name = fields[2];
        double avg=0, cost=0, fare=0, maxDist=0;
        try { avg = stod(fields[3]); cost = stod(fields[4]); fare = stod(fields[5]); maxDist = stod(fields[6]); }
        catch (const exception& e) { cerr << "[TransportOptionCsvLoader Warning] Line " << lineNumber << ": Number parsing error - " << e.what() << " - Skipping row" << endl; continue; }
        auto modeOpt = parseTransportMode(modeStr);
        if (!modeOpt) { cerr << "[TransportOptionCsvLoader Warning] Line " << lineNumber << ": Unknown transport mode '" << modeStr << "' - Skipping row" << endl; continue; }
        if (seenIds.find(id) != seenIds.end()) { cerr << "[TransportOptionCsvLoader Warning] Line " << lineNumber << ": Duplicate optionId '" << id << "' - Skipping row" << endl; continue; }
        seenIds.insert(id);
        auto optPtr = createTransportOption(id, *modeOpt, name, avg, cost, fare, maxDist);
        if (optPtr) options.push_back(optPtr);
    }
    return options;
}

vector<string> TransportOptionCsvLoader::parseCsvLine(const string& line) { return splitCsv(line); }
