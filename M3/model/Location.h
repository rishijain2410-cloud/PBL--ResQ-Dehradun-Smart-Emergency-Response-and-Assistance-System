#ifndef M3_MODEL_LOCATION_H
#define M3_MODEL_LOCATION_H

#include <string>
#include <stdexcept>

enum class LocationType {
    HOSPITAL,
    EMERGENCY_SERVICE,
    TRANSPORT_HUB,
    LANDMARK,
    UNKNOWN
};

std::string locationTypeToString(LocationType type);
LocationType stringToLocationType(const std::string& typeStr);

class Location {
private:
    std::string locationId;
    std::string name;
    double latitude;
    double longitude;
    LocationType locationType;
    std::string address;

    void validate() const;

public:
    Location();
    Location(const std::string& id,
             const std::string& name,
             double lat,
             double lon,
             LocationType type,
             const std::string& addr = "");

    // Getters
    const std::string& getId() const { return locationId; }
    const std::string& getName() const { return name; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }
    LocationType getType() const { return locationType; }
    std::string getTypeString() const { return locationTypeToString(locationType); }
    const std::string& getAddress() const { return address; }

    // Setters
    void setId(const std::string& id);
    void setName(const std::string& name);
    void setCoordinates(double lat, double lon);
    void setType(LocationType type);
    void setAddress(const std::string& addr);

    // Helpers
    bool isHospital() const { return locationType == LocationType::HOSPITAL; }
    bool isEmergencyService() const { return locationType == LocationType::EMERGENCY_SERVICE; }
    std::string toJson() const;
};

#endif // M3_MODEL_LOCATION_H
