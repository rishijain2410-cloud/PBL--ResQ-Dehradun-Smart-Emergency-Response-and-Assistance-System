#include "model/Location.h"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

bool Location::isValidLocationId(const string& id) {
    // Expected format: "LOC-<digits>" with at least one digit (minimum length 5: "LOC-1")
    if (id.length() < 5) {
        return false;
    }
    if (id.rfind("LOC-", 0) != 0) {
        return false;
    }
    for (size_t i = 4; i < id.length(); ++i) {
        if (!isdigit(static_cast<unsigned char>(id[i]))) {
            return false;
        }
    }
    return true;
}

Location::Location()
    : locationId(""),
      name(""),
      latitude(0.0),
      longitude(0.0),
      locationType(LocationType::LANDMARK),
      address("") {}

Location::Location(const string& locationId,
                   const string& name,
                   double latitude,
                   double longitude,
                   LocationType locationType,
                   const string& address)
    : locationId(locationId),
      name(name),
      latitude(latitude),
      longitude(longitude),
      locationType(locationType),
      address(address) {

    // 1. Validate ID format
    if (locationId.empty() || !isValidLocationId(locationId)) {
        throw invalid_argument(
            "Validation Error: locationId '" + locationId +
            "' is invalid. ID must follow the pattern 'LOC-<digits>' (e.g., 'LOC-001')."
        );
    }

    // 2. Validate Name
    if (name.empty()) {
        throw invalid_argument("Validation Error: Location name cannot be empty.");
    }

    // 3. Validate Latitude [-90, 90]
    if (latitude < -90.0 || latitude > 90.0) {
        ostringstream ss;
        ss << "Validation Error: Latitude " << latitude
           << " is out of bounds. Must be in range [-90.0, 90.0].";
        throw invalid_argument(ss.str());
    }

    // 4. Validate Longitude [-180, 180]
    if (longitude < -180.0 || longitude > 180.0) {
        ostringstream ss;
        ss << "Validation Error: Longitude " << longitude
           << " is out of bounds. Must be in range [-180.0, 180.0].";
        throw invalid_argument(ss.str());
    }
}

const string& Location::getLocationId() const {
    return locationId;
}

const string& Location::getName() const {
    return name;
}

double Location::getLatitude() const {
    return latitude;
}

double Location::getLongitude() const {
    return longitude;
}

LocationType Location::getLocationType() const {
    return locationType;
}

const string& Location::getAddress() const {
    return address;
}

string Location::toString() const {
    ostringstream ss;
    ss << "[" << locationId << "] " << name
       << " (" << locationTypeToString(locationType) << ") "
    << "at (" << fixed << setprecision(4) << latitude << ", "
    << fixed << setprecision(4) << longitude << ")";
    if (!address.empty()) {
        ss << " - " << address;
    }
    return ss.str();
}

string Location::locationTypeToString(LocationType type) {
    switch (type) {
        case LocationType::LANDMARK:          return "LANDMARK";
        case LocationType::HOSPITAL:          return "HOSPITAL";
        case LocationType::EMERGENCY_SERVICE: return "EMERGENCY_SERVICE";
        case LocationType::TOURIST_PLACE:     return "TOURIST_PLACE";
        case LocationType::HOTEL:             return "HOTEL";
        case LocationType::RESTAURANT:        return "RESTAURANT";
        case LocationType::TRANSPORT_HUB:     return "TRANSPORT_HUB";
        default:                              return "UNKNOWN";
    }
}

LocationType Location::stringToLocationType(const string& typeStr) {
    // Normalization: trim whitespace and uppercase
    string s = typeStr;
    // Trim leading whitespace
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    // Trim trailing whitespace
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());

    string upperStr;
    upperStr.reserve(s.size());
    for (char c : s) {
        upperStr.push_back(static_cast<char>(toupper(static_cast<unsigned char>(c))));
    }

    if (upperStr == "LANDMARK")          return LocationType::LANDMARK;
    if (upperStr == "HOSPITAL")          return LocationType::HOSPITAL;
    if (upperStr == "EMERGENCY_SERVICE") return LocationType::EMERGENCY_SERVICE;
    if (upperStr == "TOURIST_PLACE")     return LocationType::TOURIST_PLACE;
    if (upperStr == "HOTEL")             return LocationType::HOTEL;
    if (upperStr == "RESTAURANT")        return LocationType::RESTAURANT;
    if (upperStr == "TRANSPORT_HUB")     return LocationType::TRANSPORT_HUB;

    throw invalid_argument(
        "Validation Error: Unknown LocationType string '" + typeStr +
        "'. Supported types: LANDMARK, HOSPITAL, EMERGENCY_SERVICE, TOURIST_PLACE, HOTEL, RESTAURANT, TRANSPORT_HUB."
    );
}

bool Location::operator==(const Location& other) const {
    return locationId == other.locationId;
}

bool Location::operator!=(const Location& other) const {
    return !(*this == other);
}
