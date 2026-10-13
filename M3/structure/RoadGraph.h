#pragma once

#include "structure/LocationNode.h"
#include "structure/RoadEdge.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>

/**
 * ============================================================================
 * RoadGraph: Graph stored as adjacency list
 *
 * Space Complexity:
 * - O(V + E): Stores |V| vertices in itemMapById and |E| directed edge records
 *   across adjacencyList buckets.
 *
 * Computational Complexities:
 * - getNeighbors(nodeId): O(1) average time hash lookup.
 * - addEdge(edge, bidirectional): O(1) average time hash insertions into uniqueIdSet
 *   and amortized append to adjacencyList.
 * - addNode(node): O(1) average time insertion.
 * - hasNode(nodeId): O(1) average time hash lookup.
 * - getNode(nodeId): O(1) average time hash lookup.
 * - nodeCount(): O(1) constant time.
 * - edgeCount(): O(1) constant time (tracks unique edges in uniqueIdSet).
 * - findNearestNodeId(lat, lon): O(V) linear spatial scan using haversineDistanceKm.
 * - isConnected(): O(V + E) Breadth-First Search (BFS) traversal using requestQueue.
 *
 * OOP Demonstration:
 * - Encapsulation: adjacencyList and itemMapById are strictly private; graph
 *   topological integrity is strictly guarded by public operations.
 * ============================================================================
 */
class RoadGraph {
private:
    std::unordered_map<std::string, LocationNode> itemMapById;
    std::unordered_map<std::string, std::vector<RoadEdge>> adjacencyList;
    std::unordered_set<std::string> uniqueIdSet; // Tracks unique edge IDs to reject duplicates

public:
    RoadGraph() = default;

    /**
     * Registers a new vertex/node in the graph.
     * Rejects duplicate node IDs by throwing std::invalid_argument.
     *
     * @param node The LocationNode to insert.
     * @throws std::invalid_argument if a node with node.getNodeId() already exists.
     */
    void addNode(const LocationNode& node);

    /**
     * Adds an edge between existing nodes.
     * If bidirectional is true, also registers the reverse edge in the destination
     * node's adjacency list, sharing the same edgeId.
     *
     * @param edge The RoadEdge to register.
     * @param bidirectional When true, registers edge in both directions.
     * @throws std::invalid_argument if fromNodeId or toNodeId does not exist,
     *         or if edgeId is a duplicate.
     */
    void addEdge(const RoadEdge& edge, bool bidirectional = true);

    /**
     * Checks if a node with the given ID exists.
     * @param nodeId The unique node ID.
     * @return true if the node exists, false otherwise.
     */
    bool hasNode(const std::string& nodeId) const;

    /**
     * Retrieves a reference to the LocationNode by ID.
     * @param nodeId Unique identifier.
     * @return Const reference to the LocationNode.
     * @throws std::out_of_range with a descriptive message if node is missing.
     */
    const LocationNode& getNode(const std::string& nodeId) const;

    /**
     * Returns the outgoing edges from the specified node.
     * @param nodeId The source node ID.
     * @return Const reference to vector of outgoing RoadEdges.
     * @throws std::out_of_range if nodeId does not exist in graph.
     */
    const std::vector<RoadEdge>& getNeighbors(const std::string& nodeId) const;

    /**
     * Returns total number of vertices |V|.
     */
    std::size_t nodeCount() const;

    /**
     * Returns total number of distinct edges |E| (counts bidirectional edges once).
     */
    std::size_t edgeCount() const;

    /**
     * Finds the nearest graph vertex to an arbitrary GPS coordinate.
     * Linear scan over all vertices using haversineDistanceKm.
     *
     * @param lat Latitude in degrees [-90, 90]
     * @param lon Longitude in degrees [-180, 180]
     * @return nodeId of the closest graph node.
     * @throws std::runtime_error if the graph has 0 nodes.
     */
    std::string findNearestNodeId(double lat, double lon) const;

    /**
     * Verifies if the road graph is fully connected.
     * Executes a BFS traversal using a queue named requestQueue and visited set.
     *
     * @return true if all vertices are reachable from an arbitrary start node, false otherwise.
     */
    bool isConnected() const;

    /**
     * Retrieves all registered vertex IDs in the graph.
     * Const-only query accessor.
     * @return std::vector<std::string> containing all node IDs.
     */
    std::vector<std::string> getAllNodeIds() const;

    /**
     * Resets the graph to an empty state.
     */
    void clear();
};
