#pragma once

#include <string>
#include <stdexcept>

/**
 * Supported road classification types within the Dehradun transport network.
 */
enum class RoadType {
    HIGHWAY,
    MAIN_ROAD,
    CITY_STREET,
    HILL_ROAD
};

/**
 * ============================================================================
 * OOP Concept: Encapsulation & Edge Invariants
 * - RoadEdge represents a directed or bidirectional edge in the RoadGraph.
 * - Invariant: distanceKm > 0. Non-negative weights are strictly required
 *   by Dijkstra's shortest path algorithm to guarantee convergence and optimality.
 * - Invariant: fromNodeId != toNodeId. Disallows self-loop edges.
 * - Invariant: edgeId, fromNodeId, toNodeId, roadName must be non-empty strings.
 *
 * DSA Role:
 * - Graph Edge: Stored in adjacency lists (adjacencyList) representing weighted
 *   connectivity between LocationNode vertices.
 * ============================================================================
 */
class RoadEdge {
private:
    std::string edgeId;
    std::string fromNodeId;
    std::string toNodeId;
    std::string roadName;
    double distanceKm;
    RoadType roadType;

public:
    // Default constructor for container / vector initialization
    RoadEdge();

    // Primary constructor enforcing validation invariants
    RoadEdge(const std::string& edgeId,
             const std::string& fromNodeId,
             const std::string& toNodeId,
             const std::string& roadName,
             double distanceKm,
             RoadType roadType);

    // Getters
    const std::string& getEdgeId() const;
    const std::string& getFromNodeId() const;
    const std::string& getToNodeId() const;
    const std::string& getRoadName() const;
    double getDistanceKm() const;
    RoadType getRoadType() const;

    // String formatting helper
    std::string toString() const;

    // Static enum <-> string conversion helpers
    static std::string roadTypeToString(RoadType type);
    static RoadType stringToRoadType(const std::string& typeStr);

    // Equality operator
    bool operator==(const RoadEdge& other) const;
    bool operator!=(const RoadEdge& other) const;
};
