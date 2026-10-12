#pragma once

#include <string>
#include <stdexcept>

/**
 * ============================================================================
 * OOP Concept: Encapsulation
 * - State variables (locationId, name, latitude, longitude, locationType, address)
 *   are strictly private, hiding internal representation.
 * - Constructor enforces domain invariants (range bounds [-90, 90], [-180, 180],
 *   non-empty names, and strictly formatted identifiers "LOC-<digits>").
 * - Read-only access is provided through const-qualified getters, preventing
 *   external tampering and guaranteeing object validity throughout its lifecycle.
 *
 * DSA Role:
 * - Serves as the fundamental vertex/node entity for spatial and graph operations
 *   (e.g., RoadGraph, LocationNode) and as the primary value stored in hash-indexed
 *   structures (itemMapById).
 *
 * Time Complexity:
 * - Constructor: O(L) where L is the length of locationId (validating pattern).
 * - Accessors (getters): O(1) constant time.
 * - toString(): O(L + N + A) where N is name length and A is address length.
 * ============================================================================
 */

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

    // Helper to validate the canonical ID pattern: "LOC-<digits>" (e.g. LOC-001)
    static bool isValidLocationId(const std::string& id);

public:
    // Default constructor (initializes to an unassigned placeholder state)
    Location();

    // Parameterized constructor enforcing domain invariants
    Location(const std::string& locationId,
             const std::string& name,
             double latitude,
             double longitude,
             LocationType locationType,
             const std::string& address = "");

    // Getters
    const std::string& getLocationId() const;
    const std::string& getName() const;
    double getLatitude() const;
    double getLongitude() const;
    LocationType getLocationType() const;
    const std::string& getAddress() const;

    // Formatted string representation
    std::string toString() const;

    // Static enum <-> string conversion helpers
    static std::string locationTypeToString(LocationType type);
    static LocationType stringToLocationType(const std::string& typeStr);

    // Equality operators (equality by unique locationId)
    bool operator==(const Location& other) const;
    bool operator!=(const Location& other) const;
};
