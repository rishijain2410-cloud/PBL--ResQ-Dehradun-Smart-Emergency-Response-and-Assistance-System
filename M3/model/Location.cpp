#include "Location.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

std::string locationTypeToString(LocationType type) {
    switch (type) {
        case LocationType::HOSPITAL: return "HOSPITAL";
        case LocationType::EMERGENCY_SERVICE: return "EMERGENCY_SERVICE";
        case LocationType::TRANSPORT_HUB: return "TRANSPORT_HUB";
        case LocationType::LANDMARK: return "LANDMARK";
        default: return "UNKNOWN";
    }
}

LocationType stringToLocationType(const std::string& typeStr) {
    std::string s = typeStr;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s == "HOSPITAL") return LocationType::HOSPITAL;
    if (s == "EMERGENCY_SERVICE" || s == "EMERGENCY") return LocationType::EMERGENCY_SERVICE;
    if (s == "TRANSPORT_HUB" || s == "TRANSIT") return LocationType::TRANSPORT_HUB;
    if (s == "LANDMARK") return LocationType::LANDMARK;
    return LocationType::UNKNOWN;
}

Location::Location()
    : locationId(""), name(""), latitude(0.0), longitude(0.0),
      locationType(LocationType::UNKNOWN), address("") {}

Location::Location(const std::string& id,
                   const std::string& name,
                   double lat,
                   double lon,
                   LocationType type,
                   const std::string& addr)
    : locationId(id), name(name), latitude(lat), longitude(lon),
      locationType(type), address(addr) {
    validate();
}

void Location::validate() const {
    if (locationId.empty()) {
        throw std::invalid_argument("Location ID cannot be empty");
    }
    if (name.empty()) {
        throw std::invalid_argument("Location name cannot be empty");
    }
    if (latitude < -90.0 || latitude > 90.0) {
        throw std::invalid_argument("Latitude must be within [-90.0, 90.0]");
    }
    if (longitude < -180.0 || longitude > 180.0) {
        throw std::invalid_argument("Longitude must be within [-180.0, 180.0]");
    }
}

void Location::setId(const std::string& id) {
    if (id.empty()) throw std::invalid_argument("Location ID cannot be empty");
    locationId = id;
}

void Location::setName(const std::string& n) {
    if (n.empty()) throw std::invalid_argument("Location name cannot be empty");
    name = n;
}

void Location::setCoordinates(double lat, double lon) {
    if (lat < -90.0 || lat > 90.0) throw std::invalid_argument("Invalid latitude");
    if (lon < -180.0 || lon > 180.0) throw std::invalid_argument("Invalid longitude");
    latitude = lat;
    longitude = lon;
}

void Location::setType(LocationType type) {
    locationType = type;
}

void Location::setAddress(const std::string& addr) {
    address = addr;
}

static std::string escapeJson(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

std::string Location::toJson() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{"
       << "\"id\":\"" << escapeJson(locationId) << "\","
       << "\"name\":\"" << escapeJson(name) << "\","
       << "\"latitude\":" << latitude << ","
       << "\"longitude\":" << longitude << ","
       << "\"type\":\"" << locationTypeToString(locationType) << "\","
       << "\"address\":\"" << escapeJson(address) << "\""
       << "}";
    return ss.str();
}
