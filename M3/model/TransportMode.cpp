#include "model/TransportMode.h"

#include <algorithm>
#include <cctype>

using namespace std;

string toString(TransportMode mode) {
    switch (mode) {
        case TransportMode::AMBULANCE: return "AMBULANCE";
        case TransportMode::CAR:       return "CAR";
    }
    return "UNKNOWN";
}

string transportModeToString(TransportMode mode) {
    return toString(mode);
}

optional<TransportMode> parseTransportMode(const string& modeStr) {
    string s = modeStr;

    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char c) {
        return !isspace(c);
    }));
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char c) {
        return !isspace(c);
    }).base(), s.end());

    try {
        return fromString(s);
    } catch (const invalid_argument&) {
        return nullopt;
    }
}

TransportMode fromString(const string& modeStr) {
    string upper;
    upper.reserve(modeStr.size());
    for (char c : modeStr) {
        upper.push_back(static_cast<char>(toupper(static_cast<unsigned char>(c))));
    }

    if (upper == "AMBULANCE") return TransportMode::AMBULANCE;
    if (upper == "CAR") return TransportMode::CAR;
    throw invalid_argument("Unknown transport mode: " + modeStr);
}
