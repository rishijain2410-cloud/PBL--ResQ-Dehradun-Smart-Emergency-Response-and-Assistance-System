#pragma once

#include <string>
#include <stdexcept>

enum class LocationType {
    LANDMARK,
    HOSPITAL,
    EMERGENCY_SERVICE,
    TOURIST_PLACE,
    HOTEL,
    RESTAURANT,
    TRANSPORT_HUB
};

class Location {
private:
    std::string locationId;
    std::string name;
    double latitude;
    double longitude;
    LocationType locationType;
    std::string address;

    static bool isValidLocationId(const std::string& id);

public:
    Location();

    Location(const std::string& locationId,
             const std::string& name,
             double latitude,
             double longitude,
             LocationType locationType,
             const std::string& address = "");

    const std::string& getLocationId() const;
    const std::string& getName() const;
    double getLatitude() const;
    double getLongitude() const;
    LocationType getLocationType() const;
    const std::string& getAddress() const;

    std::string toString() const;

    static std::string locationTypeToString(LocationType type);
    static LocationType stringToLocationType(const std::string& typeStr);

    bool operator==(const Location& other) const;
    bool operator!=(const Location& other) const;
};
