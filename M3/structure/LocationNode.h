#pragma once

#include "model/Location.h"
#include <string>

/**
 * ============================================================================
 * OOP Concept: Encapsulation & Value Object
 * - LocationNode represents a vertex in the RoadGraph structure.
 * - Encapsulates geographic coordinate state and identifier immutability.
 * - Created from domain Location entities via the static factory method
 *   LocationNode::fromLocation(const Location&).
 *
 * DSA Role:
 * - Graph Vertex: Serves as the fundamental vertex stored in RoadGraph's
 *   itemMapById table and referenced by RoadEdge adjacency lists.
 * ============================================================================
 */
class LocationNode {
private:
    std::string nodeId;
    std::string name;
    double latitude;
    double longitude;

public:
    // Default constructor for map/container storage
    LocationNode();

    // Parameterized constructor
    LocationNode(const std::string& nodeId,
                 const std::string& name,
                 double latitude,
                 double longitude);

    // Static factory creating a graph node from a domain Location entity
    static LocationNode fromLocation(const Location& location);

    // Getters
    const std::string& getNodeId() const;
    const std::string& getName() const;
    double getLatitude() const;
    double getLongitude() const;

    // String representation helper
    std::string toString() const;

    // Equality operators (equality determined by nodeId)
    bool operator==(const LocationNode& other) const;
    bool operator!=(const LocationNode& other) const;
};
