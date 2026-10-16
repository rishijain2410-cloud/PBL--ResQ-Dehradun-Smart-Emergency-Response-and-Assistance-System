#include "model/Location.h"
#include "repository/LocationRepository.h"
#include "datasource/LocationCsvLoader.h"
#include "datasource/RoadNetworkCsvLoader.h"
#include "service/RoadGraphBuilder.h"
#include "service/ShortestPathFinder.h"
#include "structure/LocationNode.h"
#include "structure/RoadEdge.h"
#include "structure/RoadGraph.h"
#include "dto/ApiResponse.h"
#include "dto/RouteSegment.h"
#include "dto/RouteResponse.h"
#include "util/GeoUtil.h"

// Task 4 includes
#include "model/TransportMode.h"
#include "model/TransportOption.h"
#include "model/AmbulanceOption.h"
#include "model/CarOption.h"
#include "dto/RouteRequest.h"
#include "dto/TransportEstimate.h"
#include "service/RouteRequestValidator.h"
#include "service/TransportEstimator.h"
#include "repository/TransportOptionRepository.h"
#include "datasource/TransportOptionCsvLoader.h"

// Task 5 includes
#include "service/NearbyServicesFinder.h"

// Task 6 includes
#include "service/RestApiServer.h"
#include "httplib.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <limits>

static int passedTests = 0;
static int totalTests = 0;

#define TEST_ASSERT(cond, msg) do { \
    totalTests++; \
    if (cond) { \
        passedTests++; \
    } else { \
        std::cerr << "FAIL: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while(0)

void testLocationValidation() {
    std::cout << "[Test Suite] Location Domain Model & Validation\n";

    try {
        Location loc("LOC-001", "Clock Tower", 30.3244, 78.0418, LocationType::LANDMARK, "Rajpur Road");
        TEST_ASSERT(loc.getLocationId() == "LOC-001", "Valid location ID matches");
        TEST_ASSERT(loc.getName() == "Clock Tower", "Valid location name matches");
        TEST_ASSERT(std::abs(loc.getLatitude() - 30.3244) < 1e-6, "Valid latitude matches");
        TEST_ASSERT(std::abs(loc.getLongitude() - 78.0418) < 1e-6, "Valid longitude matches");
        TEST_ASSERT(loc.getLocationType() == LocationType::LANDMARK, "Valid location type matches");
        TEST_ASSERT(loc.getAddress() == "Rajpur Road", "Valid address matches");
        TEST_ASSERT(!loc.toString().empty(), "toString produces non-empty text");
    } catch (...) {
        TEST_ASSERT(false, "Valid Location threw unexpected exception");
    }

    bool threw = false;
    try { Location loc("LOC-", "Test", 30.0, 78.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "ID 'LOC-' without digits must throw std::invalid_argument");

    threw = false;
    try { Location loc("LOC-ABC", "Test", 30.0, 78.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "ID 'LOC-ABC' with non-digits must throw std::invalid_argument");

    threw = false;
    try { Location loc("ABC-123", "Test", 30.0, 78.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "ID without 'LOC-' prefix must throw std::invalid_argument");

    threw = false;
    try { Location loc("LOC-101", "", 30.0, 78.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Empty name must throw std::invalid_argument");

    threw = false;
    try { Location loc("LOC-101", "Test", 95.0, 78.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Latitude > 90 must throw std::invalid_argument");

    threw = false;
    try { Location loc("LOC-101", "Test", -90.5, 78.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Latitude < -90 must throw std::invalid_argument");

    threw = false;
    try { Location loc("LOC-101", "Test", 30.0, 185.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Longitude > 180 must throw std::invalid_argument");

    threw = false;
    try { Location loc("LOC-101", "Test", 30.0, -185.0, LocationType::LANDMARK); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Longitude < -180 must throw std::invalid_argument");

    TEST_ASSERT(Location::locationTypeToString(LocationType::HOSPITAL) == "HOSPITAL", "Enum to string HOSPITAL");
    TEST_ASSERT(Location::stringToLocationType("HOSPITAL") == LocationType::HOSPITAL, "String to enum HOSPITAL");
    TEST_ASSERT(Location::stringToLocationType("  tourist_place  ") == LocationType::TOURIST_PLACE, "Case and trim insensitive enum parsing");

    threw = false;
    try { Location::stringToLocationType("INVALID_TYPE"); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Invalid enum string must throw std::invalid_argument");
}

void testGeoUtil() {
    std::cout << "[Test Suite] GeoUtil Haversine Distance Calculation\n";

    double zeroDist = GeoUtil::haversineDistanceKm(30.3244, 78.0418, 30.3244, 78.0418);
    TEST_ASSERT(zeroDist == 0.0, "Distance between identical coordinates is 0.0");

    double distAB = GeoUtil::haversineDistanceKm(30.3244, 78.0418, 30.3155, 78.0350);
    double distBA = GeoUtil::haversineDistanceKm(30.3155, 78.0350, 30.3244, 78.0418);
    TEST_ASSERT(std::abs(distAB - distBA) < 1e-9, "Haversine distance is symmetric");
    TEST_ASSERT(distAB > 0.5 && distAB < 3.0, "Clock Tower to Railway Station distance is realistic (~1.2 km)");
}

void testInMemoryRepository() {
    std::cout << "[Test Suite] InMemoryLocationRepository\n";

    InMemoryLocationRepository repo;
    Location loc1("LOC-001", "Clock Tower", 30.3244, 78.0418, LocationType::LANDMARK, "Rajpur Road");
    Location loc2("LOC-002", "ISBT", 30.2874, 78.0069, LocationType::TRANSPORT_HUB, "Haridwar Bypass");
    Location loc3("LOC-003", "Doon Hospital", 30.3235, 78.0450, LocationType::HOSPITAL, "Court Road");

    TEST_ASSERT(repo.save(loc1) == true, "Save loc1 succeeds");
    TEST_ASSERT(repo.save(loc2) == true, "Save loc2 succeeds");
    TEST_ASSERT(repo.save(loc3) == true, "Save loc3 succeeds");

    Location locDuplicate("LOC-001", "Duplicate Ghanta Ghar", 30.32, 78.04, LocationType::LANDMARK);
    TEST_ASSERT(repo.save(locDuplicate) == false, "Duplicate ID must be rejected");
    TEST_ASSERT(repo.size() == 3, "Size remains 3 after duplicate rejection");

    auto found1 = repo.findById("LOC-001");
    TEST_ASSERT(found1.has_value() && found1->getName() == "Clock Tower", "findById returns correct location");

    auto notFound = repo.findById("LOC-999");
    TEST_ASSERT(!notFound.has_value(), "findById returns nullopt for non-existent ID");

    auto all = repo.findAll();
    TEST_ASSERT(all.size() == 3, "findAll returns all 3 elements");

    auto hospitals = repo.findByType(LocationType::HOSPITAL);
    TEST_ASSERT(hospitals.size() == 1 && hospitals[0].getLocationId() == "LOC-003", "findByType filters correctly");
}

void testApiResponse() {
    std::cout << "[Test Suite] ApiResponse DTO Envelope\n";

    Location loc("LOC-001", "Clock Tower", 30.3244, 78.0418, LocationType::LANDMARK);
    auto successResp = ApiResponse<Location>::ok(loc, "Found");
    TEST_ASSERT(successResp.success == true, "ApiResponse::ok sets success = true");
    TEST_ASSERT(successResp.errorCode.empty(), "ApiResponse::ok sets empty errorCode");

    auto failResp = ApiResponse<Location>::fail("Not found", ErrorCode::NOT_FOUND);
    TEST_ASSERT(failResp.success == false, "ApiResponse::fail sets success = false");
    TEST_ASSERT(failResp.errorCode == "NOT_FOUND", "ApiResponse::fail sets NOT_FOUND error code");
}

void testCsvLoaderWithMalformedRows() {
    std::cout << "[Test Suite] LocationCsvLoader Error Handling\n";

    std::string csvData =
        "# Comment header\n"
        "locationId,name,latitude,longitude,locationType,address\n"
        "LOC-001,Clock Tower,30.3244,78.0418,LANDMARK,Rajpur Road\n"
        "\n"
        "MALFORMED_LINE_TOO_FEW_COLUMNS\n"
        "LOC-002,Invalid Lat,150.000,78.0069,TRANSPORT_HUB,ISBT\n"
        "LOC-NOT-DIGITS,Invalid ID,30.25,78.01,HOTEL,Address\n"
        "LOC-003,Unknown Type,30.31,78.03,ALIEN_TYPE,Address\n"
        "LOC-004,Valid Hub,30.3155,78.0350,TRANSPORT_HUB,Railway Colony\n";

    std::istringstream stream(csvData);
    std::vector<Location> results = LocationCsvLoader::loadFromStream(stream);

    TEST_ASSERT(results.size() == 2, "CsvLoader loaded 2 valid rows while gracefully skipping malformed rows");
}

void testLocationNode() {
    std::cout << "[Test Suite] LocationNode Structure\n";

    Location loc("LOC-001", "Clock Tower", 30.3244, 78.0418, LocationType::LANDMARK, "Center");
    LocationNode node = LocationNode::fromLocation(loc);

    TEST_ASSERT(node.getNodeId() == "LOC-001", "Factory creates node with matching nodeId");
    TEST_ASSERT(node.getName() == "Clock Tower", "Factory creates node with matching name");
    TEST_ASSERT(std::abs(node.getLatitude() - 30.3244) < 1e-6, "Factory creates node with matching latitude");
    TEST_ASSERT(std::abs(node.getLongitude() - 78.0418) < 1e-6, "Factory creates node with matching longitude");

    LocationNode copyNode("LOC-001", "Other Name", 0, 0);
    TEST_ASSERT(node == copyNode, "Equality operator compares by nodeId");
}

void testRoadEdgeValidation() {
    std::cout << "[Test Suite] RoadEdge Invariants & Validation\n";

    try {
        RoadEdge edge("EDG-001", "LOC-001", "LOC-002", "Rajpur Road", 1.5, RoadType::MAIN_ROAD);
        TEST_ASSERT(edge.getEdgeId() == "EDG-001", "Valid edge ID matches");
        TEST_ASSERT(edge.getFromNodeId() == "LOC-001", "Valid fromNodeId matches");
        TEST_ASSERT(edge.getToNodeId() == "LOC-002", "Valid toNodeId matches");
        TEST_ASSERT(edge.getRoadName() == "Rajpur Road", "Valid roadName matches");
        TEST_ASSERT(std::abs(edge.getDistanceKm() - 1.5) < 1e-6, "Valid distance matches");
        TEST_ASSERT(edge.getRoadType() == RoadType::MAIN_ROAD, "Valid roadType matches");
    } catch (...) {
        TEST_ASSERT(false, "Valid RoadEdge threw unexpected exception");
    }

    bool threw = false;
    try { RoadEdge e("EDG-001", "LOC-001", "LOC-002", "Road", 0.0, RoadType::MAIN_ROAD); }
    catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Zero distance throws std::invalid_argument");

    threw = false;
    try { RoadEdge e("EDG-001", "LOC-001", "LOC-002", "Road", -2.5, RoadType::MAIN_ROAD); }
    catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Negative distance throws std::invalid_argument");

    threw = false;
    try { RoadEdge e("EDG-001", "LOC-001", "LOC-001", "Road", 1.0, RoadType::CITY_STREET); }
    catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Self-loop edge (from == to) throws std::invalid_argument");

    threw = false;
    try { RoadEdge e("", "LOC-001", "LOC-002", "Road", 1.0, RoadType::CITY_STREET); }
    catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Empty edgeId throws std::invalid_argument");

    threw = false;
    try { RoadEdge e("EDG-001", "", "LOC-002", "Road", 1.0, RoadType::CITY_STREET); }
    catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Empty fromNodeId throws std::invalid_argument");

    threw = false;
    try { RoadEdge e("EDG-001", "LOC-001", "", "Road", 1.0, RoadType::CITY_STREET); }
    catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Empty toNodeId throws std::invalid_argument");

    threw = false;
    try { RoadEdge e("EDG-001", "LOC-001", "LOC-002", "", 1.0, RoadType::CITY_STREET); }
    catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Empty roadName throws std::invalid_argument");

    TEST_ASSERT(RoadEdge::roadTypeToString(RoadType::HILL_ROAD) == "HILL_ROAD", "Enum to string HILL_ROAD");
    TEST_ASSERT(RoadEdge::stringToRoadType("hill_road") == RoadType::HILL_ROAD, "String to enum hill_road");
}

void testRoadGraphNormalOperations() {
    std::cout << "[Test Suite] RoadGraph Normal Operations\n";

    RoadGraph graph;
    LocationNode n1("LOC-001", "Clock Tower", 30.3244, 78.0418);
    LocationNode n2("LOC-002", "ISBT", 30.2874, 78.0069);
    LocationNode n3("LOC-003", "Railway Station", 30.3155, 78.0350);

    graph.addNode(n1);
    graph.addNode(n2);
    graph.addNode(n3);

    TEST_ASSERT(graph.nodeCount() == 3, "Node count is 3 after additions");
    TEST_ASSERT(graph.hasNode("LOC-001") == true, "hasNode returns true for existing node");
    TEST_ASSERT(graph.hasNode("LOC-999") == false, "hasNode returns false for non-existent node");

    RoadEdge e1("EDG-001", "LOC-001", "LOC-003", "Gandhi Road", 1.5, RoadType::MAIN_ROAD);
    graph.addEdge(e1, true);

    TEST_ASSERT(graph.edgeCount() == 1, "edgeCount counts bidirectional road once");

    const auto& n1Neighbors = graph.getNeighbors("LOC-001");
    TEST_ASSERT(n1Neighbors.size() == 1, "LOC-001 has 1 neighbor");
    TEST_ASSERT(n1Neighbors[0].getToNodeId() == "LOC-003", "LOC-001 neighbor is LOC-003");
    TEST_ASSERT(n1Neighbors[0].getEdgeId() == "EDG-001", "LOC-001 neighbor has edgeId EDG-001");

    const auto& n3Neighbors = graph.getNeighbors("LOC-003");
    TEST_ASSERT(n3Neighbors.size() == 1, "LOC-003 has 1 reverse neighbor");
    TEST_ASSERT(n3Neighbors[0].getToNodeId() == "LOC-001", "Reverse edge points back to LOC-001");
    TEST_ASSERT(n3Neighbors[0].getEdgeId() == "EDG-001", "Reverse edge retains edgeId EDG-001");

    RoadEdge e2("EDG-002", "LOC-003", "LOC-002", "Saharanpur Road", 5.0, RoadType::MAIN_ROAD);
    graph.addEdge(e2, true);
    TEST_ASSERT(graph.edgeCount() == 2, "edgeCount is 2 after second edge");

    TEST_ASSERT(graph.isConnected() == true, "Graph LOC-001 - LOC-003 - LOC-002 is fully connected");
}

void testRoadGraphEdgeCases() {
    std::cout << "[Test Suite] RoadGraph Edge Cases & Boundary Conditions\n";

    RoadGraph graph;
    LocationNode n1("LOC-001", "Clock Tower", 30.3244, 78.0418);
    LocationNode n2("LOC-002", "ISBT", 30.2874, 78.0069);
    graph.addNode(n1);
    graph.addNode(n2);

    bool threw = false;
    try { graph.addNode(n1); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Adding duplicate node throws std::invalid_argument");
    TEST_ASSERT(graph.nodeCount() == 2, "Node count unaffected by duplicate node attempt");

    RoadEdge e1("EDG-001", "LOC-001", "LOC-002", "Road", 1.5, RoadType::MAIN_ROAD);
    graph.addEdge(e1, true);

    threw = false;
    try {
        RoadEdge eDup("EDG-001", "LOC-002", "LOC-001", "Other Road", 2.0, RoadType::CITY_STREET);
        graph.addEdge(eDup, true);
    } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Adding duplicate edge ID throws std::invalid_argument");
    TEST_ASSERT(graph.edgeCount() == 1, "Edge count unaffected by duplicate edge attempt");

    threw = false;
    try {
        RoadEdge eUnknown("EDG-002", "LOC-001", "LOC-999", "Ghost Road", 1.0, RoadType::CITY_STREET);
        graph.addEdge(eUnknown);
    } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Edge with unknown toNodeId throws std::invalid_argument");

    threw = false;
    try {
        RoadEdge eUnknown("EDG-003", "LOC-999", "LOC-002", "Ghost Road", 1.0, RoadType::CITY_STREET);
        graph.addEdge(eUnknown);
    } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "Edge with unknown fromNodeId throws std::invalid_argument");

    threw = false;
    try { graph.getNode("LOC-999"); } catch (const std::out_of_range&) { threw = true; }
    TEST_ASSERT(threw, "getNode on missing node throws std::out_of_range");

    threw = false;
    try { graph.getNeighbors("LOC-999"); } catch (const std::out_of_range&) { threw = true; }
    TEST_ASSERT(threw, "getNeighbors on missing node throws std::out_of_range");

    RoadGraph emptyGraph;
    threw = false;
    try { emptyGraph.findNearestNodeId(30.3, 78.0); } catch (const std::runtime_error&) { threw = true; }
    TEST_ASSERT(threw, "findNearestNodeId on empty graph throws std::runtime_error");

    LocationNode isolatedNode("LOC-003", "Isolated Node", 30.35, 78.05);
    graph.addNode(isolatedNode);
    TEST_ASSERT(graph.isConnected() == false, "isConnected on graph with isolated component returns false");
}

void testRealSeedNetwork() {
    std::cout << "[Test Suite] Real Seed Network (locations.csv & road_edges.csv)\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    TEST_ASSERT(locations.size() == 20, "20 locations loaded from seed/locations.csv");

    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    TEST_ASSERT(edges.size() == 30, "30 road edges loaded from seed/road_edges.csv");

    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    TEST_ASSERT(graph.nodeCount() == 20, "RoadGraph contains exactly 20 nodes");
    TEST_ASSERT(graph.edgeCount() == 30, "RoadGraph contains exactly 30 unique edges");
    TEST_ASSERT(graph.isConnected() == true, "Real Dehradun seed road network isConnected() == true");

    const auto& ctNeighbors = graph.getNeighbors("LOC-001");
    TEST_ASSERT(ctNeighbors.size() >= 5, "Clock Tower (LOC-001) has at least 5 arterial connections");

    std::string nearestId = graph.findNearestNodeId(30.3250, 78.0420);
    TEST_ASSERT(nearestId == "LOC-001", "Point near Clock Tower resolves to LOC-001");
}

// ============================================================================
// TASK 3 TEST SUITES: DIJKSTRA SHORTEST PATH ENGINE
// ============================================================================

void testDijkstraWeightedVsHopCount() {
    std::cout << "[Test Suite] Dijkstra: Weighted Distance vs Hop Count\n";

    // Hand-built graph where 1-hop path has weight 10, but 2-hop path has weight 3 + 3 = 6
    RoadGraph graph;
    graph.addNode(LocationNode("LOC-A", "Node A", 30.0, 78.0));
    graph.addNode(LocationNode("LOC-B", "Node B", 30.1, 78.1));
    graph.addNode(LocationNode("LOC-C", "Node C", 30.05, 78.05));

    graph.addEdge(RoadEdge("EDG-001", "LOC-A", "LOC-B", "Direct Highway", 10.0, RoadType::HIGHWAY), true);
    graph.addEdge(RoadEdge("EDG-002", "LOC-A", "LOC-C", "City Road 1", 3.0, RoadType::CITY_STREET), true);
    graph.addEdge(RoadEdge("EDG-003", "LOC-C", "LOC-B", "City Road 2", 3.0, RoadType::CITY_STREET), true);

    ShortestPathFinder finder(graph);
    ApiResponse<RouteResponse> resp = finder.findShortestRoute("LOC-A", "LOC-B");

    TEST_ASSERT(resp.success == true, "Shortest route calculation succeeds");
    TEST_ASSERT(std::abs(resp.data.getTotalDistanceKm() - 6.0) < 1e-6, "Shortest path chooses 6.0 km path over 10.0 km direct hop");
    TEST_ASSERT(resp.data.segmentCount() == 2, "Path contains 2 segments");

    const auto& path = resp.data.getPathNodeIds();
    TEST_ASSERT(path.size() == 3, "Path contains 3 nodes");
    if (path.size() == 3) {
        TEST_ASSERT(path[0] == "LOC-A" && path[1] == "LOC-C" && path[2] == "LOC-B", "Path sequence is A -> C -> B");
    }
}

void testDijkstraAlternativePaths() {
    std::cout << "[Test Suite] Dijkstra: Alternative Paths Comparison\n";

    RoadGraph graph;
    graph.addNode(LocationNode("LOC-A", "Node A", 30.0, 78.0));
    graph.addNode(LocationNode("LOC-B", "Node B", 30.1, 78.1));
    graph.addNode(LocationNode("LOC-X", "Detour X", 30.05, 78.05));
    graph.addNode(LocationNode("LOC-Y", "Shorter Y", 30.06, 78.06));

    // Path 1 via X: 4.5 + 4.5 = 9.0 km
    graph.addEdge(RoadEdge("EDG-001", "LOC-A", "LOC-X", "Route via X1", 4.5, RoadType::MAIN_ROAD), true);
    graph.addEdge(RoadEdge("EDG-002", "LOC-X", "LOC-B", "Route via X2", 4.5, RoadType::MAIN_ROAD), true);

    // Path 2 via Y: 2.1 + 3.1 = 5.2 km
    graph.addEdge(RoadEdge("EDG-003", "LOC-A", "LOC-Y", "Route via Y1", 2.1, RoadType::MAIN_ROAD), true);
    graph.addEdge(RoadEdge("EDG-004", "LOC-Y", "LOC-B", "Route via Y2", 3.1, RoadType::MAIN_ROAD), true);

    ShortestPathFinder finder(graph);
    ApiResponse<RouteResponse> resp = finder.findShortestRoute("LOC-A", "LOC-B");

    TEST_ASSERT(resp.success == true, "Alternative paths resolved");
    TEST_ASSERT(std::abs(resp.data.getTotalDistanceKm() - 5.2) < 1e-6, "Optimal path via Y (5.2 km) selected over X (9.0 km)");
    const auto& path = resp.data.getPathNodeIds();
    TEST_ASSERT(path.size() == 3 && path[1] == "LOC-Y", "Path traverses via Y");
}

void testDijkstraEdgeCases() {
    std::cout << "[Test Suite] Dijkstra: Edge Cases & Error Envelopes\n";

    RoadGraph graph;
    graph.addNode(LocationNode("LOC-001", "Node 1", 30.0, 78.0));
    graph.addNode(LocationNode("LOC-002", "Node 2", 30.1, 78.1));
    graph.addNode(LocationNode("LOC-003", "Node 3 (Isolated)", 30.2, 78.2));
    graph.addEdge(RoadEdge("EDG-001", "LOC-001", "LOC-002", "Link", 2.0, RoadType::MAIN_ROAD), true);

    ShortestPathFinder finder(graph);

    // 1. start == end
    auto selfResp = finder.findShortestRoute("LOC-001", "LOC-001");
    TEST_ASSERT(selfResp.success == true, "start == end returns success");
    TEST_ASSERT(selfResp.data.getTotalDistanceKm() == 0.0, "start == end has distance 0.0");
    TEST_ASSERT(selfResp.data.segmentCount() == 0, "start == end has 0 segments");
    TEST_ASSERT(selfResp.data.getPathNodeIds().size() == 1, "start == end path has single node");

    // 2. Unknown start
    auto unkStartResp = finder.findShortestRoute("LOC-999", "LOC-001");
    TEST_ASSERT(unkStartResp.success == false, "Unknown start returns failure");
    TEST_ASSERT(unkStartResp.errorCode == "NOT_FOUND", "Unknown start returns NOT_FOUND");

    // 3. Unknown end
    auto unkEndResp = finder.findShortestRoute("LOC-001", "LOC-999");
    TEST_ASSERT(unkEndResp.success == false, "Unknown end returns failure");
    TEST_ASSERT(unkEndResp.errorCode == "NOT_FOUND", "Unknown end returns NOT_FOUND");

    // 4. Empty start or end ID
    auto emptyStartResp = finder.findShortestRoute("", "LOC-001");
    TEST_ASSERT(emptyStartResp.success == false, "Empty start returns failure");
    TEST_ASSERT(emptyStartResp.errorCode == "INVALID_ID", "Empty start returns INVALID_ID");

    auto emptyEndResp = finder.findShortestRoute("LOC-001", "");
    TEST_ASSERT(emptyEndResp.success == false, "Empty end returns failure");
    TEST_ASSERT(emptyEndResp.errorCode == "INVALID_ID", "Empty end returns INVALID_ID");

    // 5. Disconnected destination (NO_RESULTS)
    auto discResp = finder.findShortestRoute("LOC-001", "LOC-003");
    TEST_ASSERT(discResp.success == false, "Disconnected destination returns failure");
    TEST_ASSERT(discResp.errorCode == "NO_RESULTS", "Disconnected destination returns NO_RESULTS");
}

void testDijkstraBidirectionalSymmetry() {
    std::cout << "[Test Suite] Dijkstra: Bidirectional Symmetry on Seed Graph\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    ShortestPathFinder finder(graph);

    // Test symmetry across key landmark pairs
    std::vector<std::pair<std::string, std::string>> samplePairs = {
        {"LOC-001", "LOC-002"},
        {"LOC-001", "LOC-003"},
        {"LOC-003", "LOC-006"},
        {"LOC-005", "LOC-007"},
        {"LOC-002", "LOC-004"},
        {"LOC-008", "LOC-012"}
    };

    for (const auto& [u, v] : samplePairs) {
        auto respUV = finder.findShortestRoute(u, v);
        auto respVU = finder.findShortestRoute(v, u);

        TEST_ASSERT(respUV.success == true && respVU.success == true, "Both route directions succeed");
        double diff = std::abs(respUV.data.getTotalDistanceKm() - respVU.data.getTotalDistanceKm());
        TEST_ASSERT(diff < 1e-9, "Distance is symmetric for " + u + " <-> " + v);
    }
}

void testDijkstraSegmentConnectivityAndSum() {
    std::cout << "[Test Suite] Dijkstra: Segment Connectivity & Invariant Consistency\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    ShortestPathFinder finder(graph);

    // Multi-segment route: FRI (LOC-005) -> Sahastradhara (LOC-007)
    auto resp = finder.findShortestRoute("LOC-005", "LOC-007");
    TEST_ASSERT(resp.success == true, "Route LOC-005 -> LOC-007 succeeds");

    const auto& segments = resp.data.getSegments();
    TEST_ASSERT(segments.size() >= 2, "Route consists of multiple sequential segments");

    // Check consecutive segment connectivity: seg[i].to == seg[i+1].from
    bool allConnected = true;
    for (size_t i = 0; i + 1 < segments.size(); ++i) {
        if (segments[i].getToNodeId() != segments[i + 1].getFromNodeId()) {
            allConnected = false;
            break;
        }
    }
    TEST_ASSERT(allConnected, "Consecutive segments connect seamlessly (seg[i].to == seg[i+1].from)");

    // Check endpoints match
    if (!segments.empty()) {
        TEST_ASSERT(segments.front().getFromNodeId() == "LOC-005", "First segment starts at origin LOC-005");
        TEST_ASSERT(segments.back().getToNodeId() == "LOC-007", "Final segment terminates at destination LOC-007");
    }

    // Check segment distance sum equals totalDistanceKm
    double segSum = 0.0;
    for (const auto& s : segments) {
        segSum += s.getDistanceKm();
    }
    TEST_ASSERT(std::abs(segSum - resp.data.getTotalDistanceKm()) < 1e-4, "Segment sum strictly equals totalDistanceKm");
}

// Independent Bellman-Ford implementation for cross-algorithm verification
static double bellmanFordDistance(const RoadGraph& graph,
                                  const std::string& startId,
                                  const std::string& targetId) {
    std::vector<std::string> allNodes = graph.getAllNodeIds();
    std::unordered_map<std::string, double> dist;
    for (const auto& id : allNodes) {
        dist[id] = 1e18;
    }
    dist[startId] = 0.0;

    // Relax all edges |V| - 1 times
    for (size_t step = 0; step < allNodes.size() - 1; ++step) {
        bool changed = false;
        for (const auto& u : allNodes) {
            if (dist[u] >= 1e17) continue;
            for (const auto& edge : graph.getNeighbors(u)) {
                const std::string& v = edge.getToNodeId();
                double w = edge.getDistanceKm();
                if (dist[u] + w < dist[v] - 1e-9) {
                    dist[v] = dist[u] + w;
                    changed = true;
                }
            }
        }
        if (!changed) break;
    }
    return dist[targetId];
}

void testBellmanFordAllPairsComparison() {
    std::cout << "[Test Suite] Dijkstra vs Bellman-Ford All-Pairs Comparison (380 Node Pairs)\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    ShortestPathFinder finder(graph);

    std::vector<std::string> allNodes = graph.getAllNodeIds();
    bool allMatched = true;
    size_t pairsChecked = 0;

    for (size_t i = 0; i < allNodes.size(); ++i) {
        for (size_t j = 0; j < allNodes.size(); ++j) {
            if (i == j) continue;
            pairsChecked++;

            const std::string& u = allNodes[i];
            const std::string& v = allNodes[j];

            auto dijkstraResp = finder.findShortestRoute(u, v);
            if (!dijkstraResp.success) {
                allMatched = false;
                break;
            }

            double dijkstraDist = dijkstraResp.data.getTotalDistanceKm();
            double bfDist = bellmanFordDistance(graph, u, v);

            if (std::abs(dijkstraDist - bfDist) > 1e-9) {
                allMatched = false;
                std::cerr << "Mismatch between Dijkstra (" << dijkstraDist
                          << ") and Bellman-Ford (" << bfDist << ") for " << u << " -> " << v << "\n";
                break;
            }
        }
    }

    TEST_ASSERT(pairsChecked == 380, "All 380 distinct pairs tested across 20 locations");
    TEST_ASSERT(allMatched, "Dijkstra matches independent Bellman-Ford distances within 1e-9 across all pairs");
}

void testAllRoutesExceedHaversine() {
    std::cout << "[Test Suite] Physical Geometry Invariant: Road Distance >= Haversine Distance\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    ShortestPathFinder finder(graph);

    std::vector<std::string> allNodes = graph.getAllNodeIds();
    bool allValid = true;

    for (size_t i = 0; i < allNodes.size(); ++i) {
        for (size_t j = 0; j < allNodes.size(); ++j) {
            if (i == j) continue;

            const std::string& u = allNodes[i];
            const std::string& v = allNodes[j];

            auto locU = repo.findById(u);
            auto locV = repo.findById(v);
            if (!locU.has_value() || !locV.has_value()) continue;

            double hDist = GeoUtil::haversineDistanceKm(
                locU->getLatitude(), locU->getLongitude(),
                locV->getLatitude(), locV->getLongitude()
            );

            auto routeResp = finder.findShortestRoute(u, v);
            if (routeResp.success) {
                // Allow minuscule float tolerance
                if (routeResp.data.getTotalDistanceKm() < hDist - 1e-5) {
                    allValid = false;
                    std::cerr << "Geometry Violation: Route " << u << " -> " << v
                              << " road dist " << routeResp.data.getTotalDistanceKm()
                              << " < haversine " << hDist << "\n";
                    break;
                }
            }
        }
    }

    TEST_ASSERT(allValid, "Every computed road route distance >= direct Haversine straight-line distance");
}


void testParseTransportMode() {
    std::cout << "[Test Suite] TransportMode Parsing" << std::endl;
    TEST_ASSERT(fromString("ambulance") == TransportMode::AMBULANCE,
                "fromString('ambulance') -> AMBULANCE");
    TEST_ASSERT(fromString("CAR") == TransportMode::CAR,
                "fromString('CAR') -> CAR");
    bool threw = false;
    try { fromString("bus"); } catch (const std::invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "fromString('bus') throws std::invalid_argument");
    TEST_ASSERT(!parseTransportMode("invalid").has_value(),
                "parseTransportMode('invalid') -> nullopt");
}

void testRouteRequestValidatorFailures() {
    std::cout << "[Test Suite] RouteRequestValidator Failure Cases" << std::endl;
    InMemoryLocationRepository locRepo;
    // Populate repo with a valid location
    locRepo.save(Location("LOC-001", "Clock Tower", 30.3244, 78.0418, LocationType::LANDMARK));
    locRepo.save(Location("LOC-002", "ISBT", 30.2874, 78.0069, LocationType::TRANSPORT_HUB));
    RouteRequestValidator validator(locRepo);
    // Empty source
    auto resp1 = validator.validate("", "LOC-002", "car");
    TEST_ASSERT(!resp1.success && resp1.errorCode == "INVALID_ID", "Empty source yields INVALID_ID");
    // Empty destination
    auto resp2 = validator.validate("LOC-001", "", "car");
    TEST_ASSERT(!resp2.success && resp2.errorCode == "INVALID_ID", "Empty destination yields INVALID_ID");
    // Bad ID pattern
    auto resp3 = validator.validate("LOC-XYZ", "LOC-002", "car");
    TEST_ASSERT(!resp3.success && resp3.errorCode == "INVALID_ID", "Bad source ID pattern yields INVALID_ID");
    // Non‑existent source
    auto resp4 = validator.validate("LOC-999", "LOC-002", "car");
    TEST_ASSERT(!resp4.success && resp4.errorCode == "NOT_FOUND", "Unknown source yields NOT_FOUND");
    // Non‑existent destination
    auto resp5 = validator.validate("LOC-001", "LOC-999", "car");
    TEST_ASSERT(!resp5.success && resp5.errorCode == "NOT_FOUND", "Unknown destination yields NOT_FOUND");
    // Same source and destination
    auto resp6 = validator.validate("LOC-001", "LOC-001", "car");
    TEST_ASSERT(!resp6.success && resp6.errorCode == "VALIDATION_ERROR", "Same source/dest yields VALIDATION_ERROR");
    // Unknown mode
    auto resp7 = validator.validate("LOC-001", "LOC-002", "helicopter");
    TEST_ASSERT(!resp7.success && resp7.errorCode == "VALIDATION_ERROR", "Unknown mode yields VALIDATION_ERROR");
}

void testTransportEstimator() {
    std::cout << "[Test Suite] TransportEstimator Logic" << std::endl;
    // Load transport options from CSV seed
    auto options = TransportOptionCsvLoader::loadFromFile("seed/transport_options.csv");
    InMemoryTransportOptionRepository repo;
    for (auto &opt : options) {
        repo.save(opt);
    }
    TransportEstimator estimator(repo);
    
    TEST_ASSERT(options.size() == 2, "CSV loads ambulance and car options only");
    auto ambulanceOption = repo.findByMode(TransportMode::AMBULANCE);
    auto carOption = repo.findByMode(TransportMode::CAR);
    TEST_ASSERT(ambulanceOption && ambulanceOption->getAverageSpeedKmph() == 35.0 &&
                ambulanceOption->getCostPerKm() == 0.0 && ambulanceOption->getBaseFare() == 0.0,
                "Ambulance values match CSV");
    TEST_ASSERT(carOption && carOption->getAverageSpeedKmph() == 25.0 &&
                carOption->getCostPerKm() == 12.0 && carOption->getBaseFare() == 50.0,
                "Car values match CSV");

    std::vector<RouteSegment> segments35;
    segments35.emplace_back("LOC-001", "LOC-002", "Test Road", 35.0, RoadType::CITY_STREET);
    RouteResponse ambulanceRoute("LOC-001", "LOC-002", 35.0, {"LOC-001", "LOC-002"}, segments35, 2);
    TransportEstimate ambulanceEst = estimator.estimateFor(TransportMode::AMBULANCE, ambulanceRoute);
    TEST_ASSERT(ambulanceEst.available && ambulanceEst.estimatedDurationMinutes == 60.0 &&
                ambulanceEst.estimatedCost == 0.0, "Ambulance 35 km is 60 minutes and cost 0");

    std::vector<RouteSegment> segments25;
    segments25.emplace_back("LOC-001", "LOC-002", "Test Road", 25.0, RoadType::CITY_STREET);
    RouteResponse carRoute("LOC-001", "LOC-002", 25.0, {"LOC-001", "LOC-002"}, segments25, 2);
    TransportEstimate carEst = estimator.estimateFor(TransportMode::CAR, carRoute);
    TEST_ASSERT(carEst.available && carEst.estimatedDurationMinutes == 60.0 &&
                carEst.estimatedCost == 350.0, "Car 25 km is 60 minutes and cost 350");

    std::vector<RouteSegment> segments400;
    segments400.emplace_back("LOC-001", "LOC-002", "Test Road", 400.0, RoadType::CITY_STREET);
    RouteResponse longRoute("LOC-001", "LOC-002", 400.0, {"LOC-001", "LOC-002"}, segments400, 2);
    TransportEstimate longAmbulance = estimator.estimateFor(TransportMode::AMBULANCE, longRoute);
    TEST_ASSERT(!longAmbulance.available &&
                longAmbulance.note == "Distance exceeds the practical range for this mode",
                "Ambulance 400 km is unavailable with range note");
    
    // estimateAll sorting: available options first sorted ascending duration, unavailable last
    auto allEst = estimator.estimateAll(ambulanceRoute);
    TEST_ASSERT(!allEst.empty(), "estimateAll returns at least one estimate");
    // First should be available
    TEST_ASSERT(allEst.front().available, "First estimate is available");
    // Last should be unavailable
    TEST_ASSERT(allEst.back().available, "Last estimate is available");
    // Check ascending order among available entries
    double prev = -1.0;
    for (const auto &e : allEst) {
        if (!e.available) break;
        TEST_ASSERT(e.estimatedDurationMinutes >= prev, "Durations are non-decreasing among available");
        prev = e.estimatedDurationMinutes;
    }
}


void testNearbyServicesFinder() {
    std::cout << "[Test Suite] NearbyServicesFinder" << std::endl;

    // Helper: build a small in-memory repo with known locations
    auto makeRepo = []() {
        InMemoryLocationRepository r;
        r.save(Location("LOC-001", "Clock Tower",       30.3244, 78.0418, LocationType::LANDMARK));
        r.save(Location("LOC-002", "ISBT",              30.2874, 78.0069, LocationType::TRANSPORT_HUB));
        r.save(Location("LOC-003", "Railway Station",   30.3155, 78.0350, LocationType::TRANSPORT_HUB));
        r.save(Location("LOC-011", "Doon Hospital",     30.3235, 78.0450, LocationType::HOSPITAL));
        r.save(Location("LOC-016", "Police HQ",         30.3211, 78.0425, LocationType::EMERGENCY_SERVICE));
        r.save(Location("LOC-005", "FRI",               30.3444, 77.9995, LocationType::TOURIST_PLACE));
        r.save(Location("LOC-006", "Robbers Cave",      30.3776, 78.0601, LocationType::TOURIST_PLACE));
        return r;
    };

    // a) Empty repository returns NO_RESULTS
    {
        InMemoryLocationRepository emptyRepo;
        NearbyServicesFinder f(emptyRepo);
        auto resp = f.findNearby(30.3244, 78.0418, 5.0, {});
        TEST_ASSERT(!resp.success && resp.errorCode == "NO_RESULTS", "Empty repo yields NO_RESULTS");
    }

    // b) Invalid latitude
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        auto resp = f.findNearby(91.0, 78.0418, 5.0, {});
        TEST_ASSERT(!resp.success && resp.errorCode == "INVALID_ID", "Latitude > 90 yields INVALID_ID");
        auto resp2 = f.findNearby(-91.0, 78.0418, 5.0, {});
        TEST_ASSERT(!resp2.success && resp2.errorCode == "INVALID_ID", "Latitude < -90 yields INVALID_ID");
    }

    // c) Invalid longitude
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        auto resp = f.findNearby(30.3244, 181.0, 5.0, {});
        TEST_ASSERT(!resp.success && resp.errorCode == "INVALID_ID", "Longitude > 180 yields INVALID_ID");
        auto resp2 = f.findNearby(30.3244, -181.0, 5.0, {});
        TEST_ASSERT(!resp2.success && resp2.errorCode == "INVALID_ID", "Longitude < -180 yields INVALID_ID");
    }

    // d) Invalid radius: zero, negative, absurdly large (should pass – no upper bound in spec)
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        auto resp0 = f.findNearby(30.3244, 78.0418, 0.0, {});
        TEST_ASSERT(!resp0.success && resp0.errorCode == "VALIDATION_ERROR", "Radius 0 yields VALIDATION_ERROR");
        auto respNeg = f.findNearby(30.3244, 78.0418, -1.0, {});
        TEST_ASSERT(!respNeg.success && respNeg.errorCode == "VALIDATION_ERROR", "Negative radius yields VALIDATION_ERROR");
    }

    // e) No locations within radius -> NO_RESULTS
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        // Query from a point far from all saved locations (empty fields north of Dehradun)
        // 30.0000, 78.0000 is ~3.6 km from the nearest saved location; 0.001 km finds nothing
        auto resp = f.findNearby(30.0000, 78.0000, 0.001, {});
        TEST_ASSERT(!resp.success && resp.errorCode == "NO_RESULTS", "Tiny radius yields NO_RESULTS");
    }

    // f) Single location within radius (distance correct)
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        // Search from Clock Tower with 0.5 km: Doon Hospital (LOC-011) at ~30.3235,78.0450 is very close
        double distHosp = GeoUtil::haversineDistanceKm(30.3244, 78.0418, 30.3235, 78.0450);
        auto resp = f.findNearby(30.3244, 78.0418, 0.5, {LocationType::HOSPITAL});
        TEST_ASSERT(resp.success, "Single hospital found within 0.5 km of Clock Tower");
        TEST_ASSERT(resp.data.size() == 1, "Exactly one hospital within 0.5 km");
        TEST_ASSERT(resp.data[0].getLocationId() == "LOC-011", "Correct hospital returned");
        double actualDist = GeoUtil::haversineDistanceKm(
            30.3244, 78.0418, resp.data[0].getLatitude(), resp.data[0].getLongitude());
        TEST_ASSERT(std::abs(actualDist - distHosp) < 1e-6, "Returned location distance matches hand computation");
        TEST_ASSERT(actualDist <= 0.5, "Returned location is within radius");
    }

    // g) Multiple locations, verify sort order (nearest first)
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        double qLat = 30.3244, qLon = 78.0418;
        auto resp = f.findNearby(qLat, qLon, 20.0, {});
        TEST_ASSERT(resp.success, "Multiple locations found within 20 km");
        TEST_ASSERT(resp.data.size() > 1, "More than one result returned");
        // Verify ascending distance
        double prev = -1.0;
        bool sorted = true;
        for (const auto& loc : resp.data) {
            double d = GeoUtil::haversineDistanceKm(qLat, qLon, loc.getLatitude(), loc.getLongitude());
            if (d < prev - 1e-9) { sorted = false; break; }
            prev = d;
        }
        TEST_ASSERT(sorted, "Results are sorted nearest-first");
    }

    // h) Filter by single type HOSPITAL
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        auto resp = f.findNearby(30.3244, 78.0418, 50.0, {LocationType::HOSPITAL});
        TEST_ASSERT(resp.success, "Hospital filter search succeeds");
        bool allHospitals = true;
        for (const auto& loc : resp.data) {
            if (loc.getLocationType() != LocationType::HOSPITAL) { allHospitals = false; break; }
        }
        TEST_ASSERT(allHospitals, "All results are HOSPITAL type");
    }

    // i) Filter by multiple types
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        std::vector<LocationType> multi = {LocationType::HOSPITAL, LocationType::EMERGENCY_SERVICE};
        auto resp = f.findNearby(30.3244, 78.0418, 50.0, multi);
        TEST_ASSERT(resp.success, "Multi-type filter search succeeds");
        bool onlyAllowed = true;
        for (const auto& loc : resp.data) {
            auto t = loc.getLocationType();
            if (t != LocationType::HOSPITAL && t != LocationType::EMERGENCY_SERVICE) {
                onlyAllowed = false; break;
            }
        }
        TEST_ASSERT(onlyAllowed, "All results are HOSPITAL or EMERGENCY_SERVICE");
    }

    // j) Empty filter returns all types
    {
        auto r = makeRepo();
        NearbyServicesFinder f(r);
        auto resp = f.findNearby(30.3244, 78.0418, 50.0, {});
        TEST_ASSERT(resp.success, "Empty filter search succeeds");
        // We know repo has 7 locations, all within 50 km
        TEST_ASSERT(resp.data.size() == 7, "Empty filter returns all 7 locations");
    }

    // k) Boundary: location exactly on radius boundary should be included
    {
        // Place a sentinel exactly 2.0 km from query point
        InMemoryLocationRepository r2;
        // LOC-001 at origin
        r2.save(Location("LOC-001", "Origin", 30.3244, 78.0418, LocationType::LANDMARK));
        // Compute a longitude offset that gives exactly 2.0 km (at this latitude, 1 deg lon ~ 96 km)
        // Use a point we know is > 2 km away so we can test boundary separately
        // Instead: compute actual haversine for two known points and confirm inclusion
        double qLat = 30.3244, qLon = 78.0418;
        double testLat = 30.3244;
        // Move ~2 km north: 1 degree lat ~ 111.195 km, so 2/111.195 ~ 0.017986 deg
        double testLon = qLon + (2.0 / (111.195 * std::cos(qLat * 3.14159265358979323846 / 180.0)));
        r2.save(Location("LOC-002", "Exactly2km", testLat, testLon, LocationType::LANDMARK));
        NearbyServicesFinder f2(r2);
        double actualDist = GeoUtil::haversineDistanceKm(qLat, qLon, testLat, testLon);
        auto resp = f2.findNearby(qLat, qLon, actualDist, {});
        TEST_ASSERT(resp.success, "Boundary location (exactly on radius) search succeeds");
        bool foundBoundary = false;
        for (const auto& loc : resp.data) {
            if (loc.getLocationId() == "LOC-002") { foundBoundary = true; break; }
        }
        TEST_ASSERT(foundBoundary, "Location exactly on radius boundary is included");
    }

    // k-extra) Correctness: 5 hand-chosen seed-network points, distances and sort verified
    {
        // Real seed coordinates from locations.csv:
        // Query center: Clock Tower (30.3244, 78.0418)
        // Point 1: Central Trauma Center (LOC-020: 30.3240, 78.0429) expected ~0.11 km
        // Point 2: Doon Hospital         (LOC-011: 30.3235, 78.0450) expected ~0.32 km
        // Point 3: Police HQ             (LOC-016: 30.3211, 78.0425) expected ~0.37 km
        // Point 4: Railway Station       (LOC-003: 30.3155, 78.0350) expected ~1.18 km
        // Point 5: Fire Station          (LOC-017: 30.3340, 78.0510) expected ~1.39 km
        double qLat = 30.3244, qLon = 78.0418;
        double dTrauma = GeoUtil::haversineDistanceKm(qLat, qLon, 30.3240, 78.0429);
        double dHosp   = GeoUtil::haversineDistanceKm(qLat, qLon, 30.3235, 78.0450);
        double dPolice = GeoUtil::haversineDistanceKm(qLat, qLon, 30.3211, 78.0425);
        double dRail   = GeoUtil::haversineDistanceKm(qLat, qLon, 30.3155, 78.0350);
        double dFire   = GeoUtil::haversineDistanceKm(qLat, qLon, 30.3340, 78.0510);

        TEST_ASSERT(dTrauma < 0.2, "Trauma Center is within 0.2 km of Clock Tower");
        TEST_ASSERT(dHosp < 1.0,   "Doon Hospital is within 1 km of Clock Tower");
        TEST_ASSERT(dPolice < 1.0, "Police HQ is within 1 km of Clock Tower");
        TEST_ASSERT(dRail < 2.0,   "Railway Station is within 2 km of Clock Tower");
        TEST_ASSERT(dFire < 2.0,   "Fire Station is within 2 km of Clock Tower");

        // Verify strictly ascending distance relationship between the 5 points
        TEST_ASSERT(dTrauma < dHosp,  "Point 1 (Trauma Center) nearer than Point 2 (Doon Hospital)");
        TEST_ASSERT(dHosp < dPolice,  "Point 2 (Doon Hospital) nearer than Point 3 (Police HQ)");
        TEST_ASSERT(dPolice < dRail,  "Point 3 (Police HQ) nearer than Point 4 (Railway Station)");
        TEST_ASSERT(dRail < dFire,    "Point 4 (Railway Station) nearer than Point 5 (Fire Station)");

        // Verify finder returns all 5 in exact ascending order
        InMemoryLocationRepository r5;
        r5.save(Location("LOC-001", "Clock Tower",           30.3244, 78.0418, LocationType::EMERGENCY_SERVICE));
        r5.save(Location("LOC-020", "Central Trauma Center", 30.3240, 78.0429, LocationType::EMERGENCY_SERVICE));
        r5.save(Location("LOC-011", "Doon Hospital",         30.3235, 78.0450, LocationType::HOSPITAL));
        r5.save(Location("LOC-016", "Police HQ",             30.3211, 78.0425, LocationType::EMERGENCY_SERVICE));
        r5.save(Location("LOC-003", "Railway Station",       30.3155, 78.0350, LocationType::TRANSPORT_HUB));
        r5.save(Location("LOC-017", "Fire Station",          30.3340, 78.0510, LocationType::EMERGENCY_SERVICE));

        NearbyServicesFinder f5(r5);
        auto resp = f5.findNearby(qLat, qLon, 2.0, {LocationType::HOSPITAL, LocationType::EMERGENCY_SERVICE, LocationType::TRANSPORT_HUB});
        TEST_ASSERT(resp.success, "5 hand-chosen points nearby query succeeds");
        TEST_ASSERT(resp.data.size() == 6, "All 5 candidate points plus origin returned within 2.0 km");

        // Validate ascending sort order across returned points
        bool sortOk = true;
        double prevD = -1.0;
        for (const auto& loc : resp.data) {
            double d = GeoUtil::haversineDistanceKm(qLat, qLon, loc.getLatitude(), loc.getLongitude());
            if (d < prevD - 1e-9) { sortOk = false; break; }
            prevD = d;
        }
        TEST_ASSERT(sortOk, "All 5 hand-chosen points returned in ascending distance order");
    }
}

// ============================================================================
// TASK 6 TEST SUITE: REST API ENDPOINTS
// ============================================================================

void testRestApiEndpoints() {
    std::cout << "[Test Suite] REST API Endpoints (/nearby, /route, /hospitals)\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    ShortestPathFinder pathFinder(graph);
    NearbyServicesFinder nearbyFinder(repo);

    const int testPort = 8089;
    RestApiServer server(testPort, pathFinder, nearbyFinder, repo);
    server.start();
    TEST_ASSERT(server.running(), "RestApiServer started successfully on port 8089");

    httplib::Client client("localhost", testPort);
    client.set_connection_timeout(3, 0);
    client.set_read_timeout(3, 0);

    // --- ENDPOINT 1: GET /nearby ---
    // 1. Valid request
    {
        auto res = client.Get("/nearby?lat=30.3244&lng=78.0418&radius=5.0&type=HOSPITAL");
        TEST_ASSERT(res != nullptr && res->status == 200, "GET /nearby returns HTTP 200");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": true") != std::string::npos, "/nearby JSON contains success: true");
            TEST_ASSERT(res->body.find("\"errorCode\": \"\"") != std::string::npos, "/nearby JSON contains empty errorCode");
            TEST_ASSERT(res->body.find("\"data\": [") != std::string::npos, "/nearby JSON contains data array");
            TEST_ASSERT(res->body.find("\"distance_km\"") != std::string::npos, "/nearby data items contain distance_km");
            TEST_ASSERT(res->body.find("LOC-011") != std::string::npos, "/nearby returns Doon Hospital (LOC-011)");
        }
    }

    // 2. Invalid coordinates (lat > 90) -> INVALID_ID
    {
        auto res = client.Get("/nearby?lat=95.0&lng=78.0418&radius=5.0");
        TEST_ASSERT(res != nullptr, "GET /nearby invalid lat responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": false") != std::string::npos, "/nearby invalid lat returns success: false");
            TEST_ASSERT(res->body.find("\"errorCode\": \"INVALID_ID\"") != std::string::npos, "/nearby invalid lat returns INVALID_ID");
        }
    }

    // 3. Missing radius -> VALIDATION_ERROR
    {
        auto res = client.Get("/nearby?lat=30.3244&lng=78.0418");
        TEST_ASSERT(res != nullptr, "GET /nearby missing radius responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"errorCode\": \"VALIDATION_ERROR\"") != std::string::npos, "/nearby missing radius yields VALIDATION_ERROR");
        }
    }

    // 4. Invalid radius (<= 0) -> VALIDATION_ERROR
    {
        auto res = client.Get("/nearby?lat=30.3244&lng=78.0418&radius=-1.5");
        TEST_ASSERT(res != nullptr, "GET /nearby negative radius responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"errorCode\": \"VALIDATION_ERROR\"") != std::string::npos, "/nearby negative radius yields VALIDATION_ERROR");
        }
    }

    // 5. No results in tiny radius -> NO_RESULTS
    {
        auto res = client.Get("/nearby?lat=30.0000&lng=78.0000&radius=0.001");
        TEST_ASSERT(res != nullptr, "GET /nearby tiny radius responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"errorCode\": \"NO_RESULTS\"") != std::string::npos, "/nearby empty radius yields NO_RESULTS");
        }
    }

    // 6. Multiple results sorted ascending
    {
        auto res = client.Get("/nearby?lat=30.3244&lng=78.0418&radius=10.0");
        TEST_ASSERT(res != nullptr, "GET /nearby multiple results responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": true") != std::string::npos, "/nearby 10 km search succeeds");
            TEST_ASSERT(res->body.find("LOC-011") != std::string::npos, "Contains nearby Doon Hospital");
        }
    }

    // --- ENDPOINT 2: GET /route ---
    // 1. Valid request
    {
        auto res = client.Get("/route?from=LOC-001&to=LOC-011");
        TEST_ASSERT(res != nullptr && res->status == 200, "GET /route returns HTTP 200");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": true") != std::string::npos, "/route JSON contains success: true");
            TEST_ASSERT(res->body.find("\"startNodeId\": \"LOC-001\"") != std::string::npos, "/route JSON contains startNodeId LOC-001");
            TEST_ASSERT(res->body.find("\"endNodeId\": \"LOC-011\"") != std::string::npos, "/route JSON contains endNodeId LOC-011");
            TEST_ASSERT(res->body.find("\"totalDistanceKm\"") != std::string::npos, "/route JSON contains totalDistanceKm");
            TEST_ASSERT(res->body.find("\"pathNodeIds\"") != std::string::npos, "/route JSON contains pathNodeIds");
            TEST_ASSERT(res->body.find("\"segments\"") != std::string::npos, "/route JSON contains segments");
            TEST_ASSERT(res->body.find("\"nodesExplored\"") != std::string::npos, "/route JSON contains nodesExplored");
        }
    }

    // 2. Invalid ID -> NOT_FOUND
    {
        auto res = client.Get("/route?from=LOC-001&to=LOC-999");
        TEST_ASSERT(res != nullptr, "GET /route invalid node responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": false") != std::string::npos, "/route invalid id returns success: false");
            TEST_ASSERT(res->body.find("\"errorCode\": \"NOT_FOUND\"") != std::string::npos, "/route invalid id yields NOT_FOUND");
        }
    }

    // 3. Same from/to -> distance 0.0
    {
        auto res = client.Get("/route?from=LOC-001&to=LOC-001");
        TEST_ASSERT(res != nullptr, "GET /route same node responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": true") != std::string::npos, "/route same node returns success: true");
            TEST_ASSERT(res->body.find("\"totalDistanceKm\": 0.0000") != std::string::npos, "/route same node has totalDistanceKm: 0.0000");
        }
    }

    // 4. Missing param -> INVALID_ID
    {
        auto res = client.Get("/route?from=&to=LOC-011");
        TEST_ASSERT(res != nullptr, "GET /route empty param responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"errorCode\": \"INVALID_ID\"") != std::string::npos, "/route empty param yields INVALID_ID");
        }
    }

    // --- ENDPOINT 3: GET /hospitals ---
    // 1. Valid request (all hospitals)
    {
        auto res = client.Get("/hospitals");
        TEST_ASSERT(res != nullptr && res->status == 200, "GET /hospitals returns HTTP 200");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": true") != std::string::npos, "/hospitals returns success: true");
            TEST_ASSERT(res->body.find("LOC-011") != std::string::npos, "/hospitals includes Doon Hospital");
            TEST_ASSERT(res->body.find("LOC-012") != std::string::npos, "/hospitals includes Max Hospital");
            TEST_ASSERT(res->body.find("LOC-018") != std::string::npos, "/hospitals includes Apollo Hospital");
            TEST_ASSERT(res->body.find("LOC-019") != std::string::npos, "/hospitals includes City Hospital");
        }
    }

    // 2. Filter by radius (proximity)
    {
        auto res = client.Get("/hospitals?lat=30.3244&lng=78.0418&radius=2.0");
        TEST_ASSERT(res != nullptr && res->status == 200, "GET /hospitals with radius returns HTTP 200");
        if (res) {
            TEST_ASSERT(res->body.find("\"success\": true") != std::string::npos, "/hospitals with radius returns success: true");
            TEST_ASSERT(res->body.find("LOC-011") != std::string::npos, "/hospitals within 2 km includes Doon Hospital");
        }
    }

    // 3. Proximity with no results -> NO_RESULTS
    {
        auto res = client.Get("/hospitals?lat=30.0000&lng=78.0000&radius=0.001");
        TEST_ASSERT(res != nullptr, "GET /hospitals empty area responds");
        if (res) {
            TEST_ASSERT(res->body.find("\"errorCode\": \"NO_RESULTS\"") != std::string::npos, "/hospitals empty area yields NO_RESULTS");
        }
    }

    server.stop();
    TEST_ASSERT(!server.running(), "RestApiServer stopped cleanly");
}

// ============================================================================
// TASK 9 TEST SUITES: INTEGRATION & END-TO-END TESTS
// ============================================================================

void testFullAmbulanceWorkflow() {
    std::cout << "[Test Suite] Full Ambulance Workflow (3 Accident GPS Locations)\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::string optPath = "seed/transport_options.csv";
    std::ifstream checkOpt(optPath);
    if (!checkOpt.is_open()) optPath = "m3_navigation/seed/transport_options.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    ShortestPathFinder pathFinder(graph);
    NearbyServicesFinder nearbyFinder(repo);

    auto options = TransportOptionCsvLoader::loadFromFile(optPath);
    InMemoryTransportOptionRepository optRepo;
    for (const auto& opt : options) optRepo.save(opt);
    TransportEstimator estimator(optRepo);

    // 3 realistic accident GPS locations in Dehradun
    struct AccidentLocation {
        std::string label;
        double lat;
        double lng;
    };

    std::vector<AccidentLocation> scenarios = {
        {"Clock Tower Intersection", 30.3244, 78.0418},
        {"Railway Station Area",     30.3155, 78.0350},
        {"Rajpur Road Corridor",     30.3541, 78.0702}
    };

    for (const auto& sc : scenarios) {
        // Step 1: Call findNearby to find hospitals within 5 km
        auto nearbyResp = nearbyFinder.findNearby(sc.lat, sc.lng, 5.0, {LocationType::HOSPITAL});
        TEST_ASSERT(nearbyResp.success, sc.label + " - Step 1: /nearby find hospitals within 5 km succeeds");

        // Step 2: Verify at least one hospital returned, sorted by distance
        TEST_ASSERT(!nearbyResp.data.empty(), sc.label + " - Step 2: At least one hospital found");
        bool isSorted = true;
        for (size_t i = 1; i < nearbyResp.data.size(); ++i) {
            double dPrev = GeoUtil::haversineDistanceKm(sc.lat, sc.lng, nearbyResp.data[i - 1].getLatitude(), nearbyResp.data[i - 1].getLongitude());
            double dCurr = GeoUtil::haversineDistanceKm(sc.lat, sc.lng, nearbyResp.data[i].getLatitude(), nearbyResp.data[i].getLongitude());
            if (dCurr < dPrev - 1e-9) { isSorted = false; break; }
        }
        TEST_ASSERT(isSorted, sc.label + " - Step 2: Hospitals sorted ascending by proximity");

        // Step 3: Pick first (nearest) hospital
        const Location& nearestHospital = nearbyResp.data.front();
        TEST_ASSERT(nearestHospital.getLocationType() == LocationType::HOSPITAL, sc.label + " - Step 3: Nearest is HOSPITAL type");

        // Step 4: Map accident GPS to road network node & call /route
        std::string accidentNodeId = graph.findNearestNodeId(sc.lat, sc.lng);
        auto routeResp = pathFinder.findShortestRoute(accidentNodeId, nearestHospital.getLocationId());
        TEST_ASSERT(routeResp.success, sc.label + " - Step 4: Shortest route to nearest hospital computed");

        // Step 5: Verify route exists, has segments, distance >= straight-line Haversine
        TEST_ASSERT(routeResp.data.getTotalDistanceKm() >= 0.0, sc.label + " - Step 5: Route has non-negative distance");
        double haversineDist = GeoUtil::haversineDistanceKm(sc.lat, sc.lng, nearestHospital.getLatitude(), nearestHospital.getLongitude());
        TEST_ASSERT(routeResp.data.getTotalDistanceKm() >= haversineDist - 1e-4, sc.label + " - Step 5: Road distance >= Haversine straight line");

        // Step 6: Verify realistic time estimate
        TransportEstimate carEstimate = estimator.estimateFor(TransportMode::CAR, routeResp.data);
        TEST_ASSERT(carEstimate.available, sc.label + " - Step 6: Ambulance/Car transit available");
        TEST_ASSERT(carEstimate.estimatedDurationMinutes > 0.0 && carEstimate.estimatedDurationMinutes < 60.0,
                    sc.label + " - Step 6: Time estimate is realistic (< 60 min for local city dispatch)");
    }
}

void testM1M2Integration() {
    std::cout << "[Test Suite] M1/M2 Module Integration (5 Realistic Scenarios)\n";

    std::string locPath = "seed/locations.csv";
    std::ifstream checkLoc(locPath);
    if (!checkLoc.is_open()) locPath = "m3_navigation/seed/locations.csv";

    std::string edgePath = "seed/road_edges.csv";
    std::ifstream checkEdge(edgePath);
    if (!checkEdge.is_open()) edgePath = "m3_navigation/seed/road_edges.csv";

    std::string optPath = "seed/transport_options.csv";
    std::ifstream checkOpt(optPath);
    if (!checkOpt.is_open()) optPath = "m3_navigation/seed/transport_options.csv";

    std::vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository repo;
    for (const auto& loc : locations) repo.save(loc);

    std::vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(repo, edges);
    ShortestPathFinder pathFinder(graph);
    NearbyServicesFinder nearbyFinder(repo);

    auto options = TransportOptionCsvLoader::loadFromFile(optPath);
    InMemoryTransportOptionRepository optRepo;
    for (const auto& opt : options) optRepo.save(opt);
    TransportEstimator estimator(optRepo);

    struct M1M2Scenario {
        std::string sceneName;
        double accidentLat;
        double accidentLng;
        bool firstHospitalFull; // M2 capacity flag
    };

    std::vector<M1M2Scenario> testCases = {
        {"ISBT Transit Hub Incident",             30.2874, 78.0069, false},
        {"Paltan Bazaar Market Emergency",        30.3220, 78.0392, true},
        {"Pacific Mall / Diversion Highway Crash",30.3698, 78.0772, false},
        {"Police Line Road Trauma Call",          30.3211, 78.0425, true},
        {"Ballupur Road Severe Trauma",           30.3444, 77.9995, true}
    };

    for (const auto& tc : testCases) {
        // 1. M1 calls /nearby to find hospitals within 10 km
        auto nearbyResp = nearbyFinder.findNearby(tc.accidentLat, tc.accidentLng, 10.0, {LocationType::HOSPITAL});
        TEST_ASSERT(nearbyResp.success && nearbyResp.data.size() >= 2, tc.sceneName + " - M1 /nearby returns >= 2 candidate hospitals");

        // 2. M2 checks capacity: if primary hospital full, select fallback 2nd hospital
        std::string chosenHospitalId;
        if (tc.firstHospitalFull && nearbyResp.data.size() >= 2) {
            chosenHospitalId = nearbyResp.data[1].getLocationId();
        } else {
            chosenHospitalId = nearbyResp.data[0].getLocationId();
        }
        TEST_ASSERT(!chosenHospitalId.empty(), tc.sceneName + " - M2 confirms hospital choice");

        // 3. M1 calls /route to navigate ambulance from accident point to selected hospital
        std::string originNodeId = graph.findNearestNodeId(tc.accidentLat, tc.accidentLng);
        auto routeResp = pathFinder.findShortestRoute(originNodeId, chosenHospitalId);
        TEST_ASSERT(routeResp.success, tc.sceneName + " - M1 /route returns SUCCESS");
        TEST_ASSERT(routeResp.data.getStartNodeId() == originNodeId, tc.sceneName + " - Route start matches origin node");
        TEST_ASSERT(routeResp.data.getEndNodeId() == chosenHospitalId, tc.sceneName + " - Route terminates at selected hospital");
        TEST_ASSERT(routeResp.data.getTotalDistanceKm() > 0.0 || originNodeId == chosenHospitalId,
                    tc.sceneName + " - Route distance is valid");

        // 4. Transport time estimation verified
        auto estimate = estimator.estimateFor(TransportMode::CAR, routeResp.data);
        TEST_ASSERT(estimate.available && (originNodeId == chosenHospitalId ? estimate.estimatedDurationMinutes >= 0.0 : estimate.estimatedDurationMinutes > 0.0),
                    tc.sceneName + " - Realistic ambulance transit time computed");
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << " Running M3 Navigation Automated Tests\n";
    std::cout << "========================================\n\n";

    // Task 1 & 2 Suites
    testLocationValidation();
    std::cout << "\n";
    testGeoUtil();
    std::cout << "\n";
    testInMemoryRepository();
    std::cout << "\n";
    testApiResponse();
    std::cout << "\n";
    testCsvLoaderWithMalformedRows();
    std::cout << "\n";
    testLocationNode();
    std::cout << "\n";
    testRoadEdgeValidation();
    std::cout << "\n";
    testRoadGraphNormalOperations();
    std::cout << "\n";
    testRoadGraphEdgeCases();
    std::cout << "\n";
    testRealSeedNetwork();
    std::cout << "\n";

    // Task 3 Suites
    testDijkstraWeightedVsHopCount();
    std::cout << "\n";
    testDijkstraAlternativePaths();
    std::cout << "\n";
    testDijkstraEdgeCases();
    std::cout << "\n";
    testDijkstraBidirectionalSymmetry();
    std::cout << "\n";
    testDijkstraSegmentConnectivityAndSum();
    std::cout << "\n";
    testBellmanFordAllPairsComparison();
    std::cout << "\n";
    testAllRoutesExceedHaversine();
    std::cout << "\n";

    // Task 4 Suites
    testParseTransportMode();
    std::cout << "\n";
    testRouteRequestValidatorFailures();
    std::cout << "\n";
    testTransportEstimator();
    std::cout << "\n";

    // Task 5 Suites
    testNearbyServicesFinder();
    std::cout << "\n";

    // Task 6 Suites
    testRestApiEndpoints();
    std::cout << "\n";

    // Task 9 Suites
    testFullAmbulanceWorkflow();
    std::cout << "\n";
    testM1M2Integration();

    std::cout << "\n========================================\n";
    std::cout << " Test Results: " << passedTests << " / " << totalTests << " passed.\n";
    std::cout << "========================================\n";

    return (passedTests == totalTests) ? 0 : 1;
}
