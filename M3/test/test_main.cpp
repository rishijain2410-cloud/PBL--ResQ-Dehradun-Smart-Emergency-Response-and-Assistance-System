#include "../model/Location.h"
#include "../structure/RoadGraph.h"
#include "../repository/LocationRepository.h"
#include "../datasource/LocationCsvLoader.h"
#include "../datasource/RoadNetworkCsvLoader.h"
#include "../service/RoadGraphBuilder.h"
#include "../service/ShortestPathFinder.h"
#include "../service/NearbyServicesFinder.h"
#include "../service/TransportEstimator.h"
#include "../util/GeoUtil.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

static int g_testsRun = 0;
static int g_testsPassed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_testsRun++; \
    if (cond) { \
        g_testsPassed++; \
    } else { \
        std::cerr << "  [FAIL] Assertion failed: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while(0)

static std::string locateFile(const std::string& filename) {
    std::vector<std::string> searchPaths = {
        "seed/" + filename,
        "m3_navigation/seed/" + filename,
        "../seed/" + filename,
        "../../seed/" + filename
    };
    for (const auto& p : searchPaths) {
        if (fs::exists(p)) return p;
    }
    return "seed/" + filename;
}

// 1. Domain Model Tests
void testDomainModels() {
    std::cout << "[Test Suite 1/5] Running Domain Model Unit Tests...\n";

    // Valid location
    Location loc1("LOC-TEST-1", "Test Hospital", 30.3244, 78.0418, LocationType::HOSPITAL, "Dehradun");
    TEST_ASSERT(loc1.getId() == "LOC-TEST-1", "Location ID matches");
    TEST_ASSERT(loc1.getName() == "Test Hospital", "Location name matches");
    TEST_ASSERT(loc1.isHospital() == true, "Location is hospital");
    TEST_ASSERT(loc1.getTypeString() == "HOSPITAL", "Type string matches HOSPITAL");

    // Boundary coordinate checks
    bool threwInvalidLat = false;
    try {
        Location badLat("BAD-1", "Invalid Lat", 95.0, 78.0, LocationType::LANDMARK);
    } catch (const std::invalid_argument&) {
        threwInvalidLat = true;
    }
    TEST_ASSERT(threwInvalidLat, "Latitude > 90 throws std::invalid_argument");

    bool threwInvalidLon = false;
    try {
        Location badLon("BAD-2", "Invalid Lon", 30.0, -185.0, LocationType::LANDMARK);
    } catch (const std::invalid_argument&) {
        threwInvalidLon = true;
    }
    TEST_ASSERT(threwInvalidLon, "Longitude < -180 throws std::invalid_argument");

    // Empty ID check
    bool threwEmptyId = false;
    try {
        Location emptyId("", "No ID", 30.0, 78.0, LocationType::LANDMARK);
    } catch (const std::invalid_argument&) {
        threwEmptyId = true;
    }
    TEST_ASSERT(threwEmptyId, "Empty location ID throws std::invalid_argument");

    // String-enum conversions
    TEST_ASSERT(stringToLocationType("hospital") == LocationType::HOSPITAL, "Case-insensitive type parsing");
    TEST_ASSERT(stringToLocationType("EMERGENCY") == LocationType::EMERGENCY_SERVICE, "Emergency service alias");
    TEST_ASSERT(stringToRoadType("hill_road") == RoadType::HILL_ROAD, "Hill road parsing");
    TEST_ASSERT(stringToRoadType("highway") == RoadType::HIGHWAY, "Highway parsing");
}

// 2. Graph Topology Tests
void testGraphTopology() {
    std::cout << "[Test Suite 2/5] Running Graph Topology Tests...\n";

    RoadGraph graph;
    Location locA("A", "Node A", 30.0, 78.0, LocationType::LANDMARK);
    Location locB("B", "Node B", 30.1, 78.1, LocationType::LANDMARK);
    Location locC("C", "Node C", 30.2, 78.2, LocationType::LANDMARK);

    graph.addNode(LocationNode(locA));
    graph.addNode(LocationNode(locB));
    graph.addNode(LocationNode(locC));

    TEST_ASSERT(graph.getNodeCount() == 3, "Graph contains exactly 3 nodes");
    TEST_ASSERT(graph.hasNode("A"), "Node A exists");
    TEST_ASSERT(graph.hasNode("B"), "Node B exists");
    TEST_ASSERT(!graph.hasNode("Z"), "Node Z does not exist");

    // Edge additions
    RoadEdge edgeAB("A", "B", 5.0, "Route AB", RoadType::MAIN_ROAD);
    graph.addEdge(edgeAB, true);

    TEST_ASSERT(graph.getNeighbors("A").size() == 1, "Node A has 1 neighbor");
    TEST_ASSERT(graph.getNeighbors("B").size() == 1, "Node B has 1 neighbor (bidirectional)");
    TEST_ASSERT(graph.getNeighbors("A")[0].getToNodeId() == "B", "A connects to B");
    TEST_ASSERT(graph.getNeighbors("B")[0].getToNodeId() == "A", "B connects to A");

    // Disconnected test
    TEST_ASSERT(!graph.isConnected(), "Graph is currently disconnected (Node C isolated)");

    // Connect C
    RoadEdge edgeBC("B", "C", 3.0, "Route BC", RoadType::CITY_STREET);
    graph.addEdge(edgeBC, true);
    TEST_ASSERT(graph.isConnected(), "Graph is fully connected after adding edge BC");
}

// 3. Dijkstra Pathfinding and Bellman-Ford Cross-Validation (380 pairs)
void testPathfindingAndCrossValidation() {
    std::cout << "[Test Suite 3/5] Running Pathfinding & 380-Pair Cross-Validation Suite...\n";

    std::string locFile = locateFile("locations.csv");
    std::string roadFile = locateFile("road_edges.csv");

    auto locations = LocationCsvLoader::loadFromFile(locFile);
    auto edges = RoadNetworkCsvLoader::loadFromFile(roadFile);

    TEST_ASSERT(locations.size() == 20, "Loaded 20 real Dehradun locations from CSV");
    TEST_ASSERT(edges.size() >= 30, "Loaded 30+ road edges from CSV");

    auto repo = std::make_shared<LocationRepository>();
    for (const auto& loc : locations) repo->save(loc);

    auto graph = RoadGraphBuilder::buildGraph(*repo, edges, true);
    TEST_ASSERT(graph->isConnected(), "Dehradun road graph is 100% connected");

    ShortestPathFinder pathFinder(graph);

    // Test 3.1: Identity path (0 km)
    auto ident = pathFinder.findShortestRoute("LOC-001", "LOC-001");
    TEST_ASSERT(ident.isSuccess(), "Identity route is successful");
    TEST_ASSERT(ident.getData().getTotalDistanceKm() == 0.0, "Identity distance is exactly 0.0 km");
    TEST_ASSERT(ident.getData().getPathNodeIds().size() == 1, "Identity path contains 1 node");

    // Test 3.2: Direct edge Clock Tower to Doon Hospital
    auto routeDoon = pathFinder.findShortestRoute("LOC-001", "LOC-011");
    TEST_ASSERT(routeDoon.isSuccess(), "Route to Doon Hospital calculated");
    TEST_ASSERT(std::abs(routeDoon.getData().getTotalDistanceKm() - 0.45) < 1e-4, "Direct road distance is 0.45 km");

    // Test 3.3: Segment continuity check
    auto routeMax = pathFinder.findShortestRoute("LOC-001", "LOC-004");
    TEST_ASSERT(routeMax.isSuccess(), "Route to Max Hospital calculated");
    const auto& segs = routeMax.getData().getSegments();
    for (size_t i = 1; i < segs.size(); ++i) {
        TEST_ASSERT(segs[i-1].toNodeId == segs[i].fromNodeId, "Route segments form continuous sequence");
    }

    // Test 3.4: Cross-validate Dijkstra against Bellman-Ford on ALL 20 x 19 = 380 location pairs!
    int matchingPairs = 0;
    int totalPairs = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        for (size_t j = 0; j < locations.size(); ++j) {
            if (i == j) continue;
            totalPairs++;

            std::string src = locations[i].getId();
            std::string dst = locations[j].getId();

            auto dijkstraRes = pathFinder.findShortestRoute(src, dst);
            auto bellmanRes = pathFinder.findShortestRouteBellmanFord(src, dst);

            if (dijkstraRes.isSuccess() && bellmanRes.isSuccess()) {
                double d1 = dijkstraRes.getData().getTotalDistanceKm();
                double d2 = bellmanRes.getData().getTotalDistanceKm();
                if (std::abs(d1 - d2) < 1e-5) {
                    matchingPairs++;
                }
            }
        }
    }

    TEST_ASSERT(totalPairs == 380, "Evaluated all 380 ordered location pairs");
    TEST_ASSERT(matchingPairs == 380, "100% (380/380) Dijkstra paths match Bellman-Ford identically");
    std::cout << "    -> Verified " << matchingPairs << " / " << totalPairs << " location pairs matched Bellman-Ford.\n";
}

// 4. Transport Estimator Tests
void testTransportEstimator() {
    std::cout << "[Test Suite 4/5] Running Polymorphic Transport Estimator Tests...\n";

    AmbulanceOption ambulance;
    TwoWheelerOption bike;
    WalkingOption walking;

    // Road speed differentiation
    TEST_ASSERT(ambulance.getSpeedForRoadType(RoadType::HIGHWAY) == 75.0, "Ambulance highway speed is 75 km/h");
    TEST_ASSERT(ambulance.getSpeedForRoadType(RoadType::HILL_ROAD) == 30.0, "Ambulance hill speed is 30 km/h");
    TEST_ASSERT(bike.getSpeedForRoadType(RoadType::CITY_STREET) == 38.0, "Two-wheeler city speed is 38 km/h");

    // Suitability threshold: walking rejected for > 5 km
    TEST_ASSERT(walking.isSuitable(3.0) == true, "Walking is suitable for 3 km");
    TEST_ASSERT(walking.isSuitable(8.0) == false, "Walking is rejected for 8 km");
    TEST_ASSERT(walking.calculateCost(4.0) == 0.0, "Walking cost is 0.0 rupees");
}

// 5. Nearby Services Spatial Search Tests
void testNearbySearch() {
    std::cout << "[Test Suite 5/5] Running Nearby Spatial Search Tests...\n";

    std::string locFile = locateFile("locations.csv");
    auto locations = LocationCsvLoader::loadFromFile(locFile);
    auto repo = std::make_shared<LocationRepository>();
    for (const auto& loc : locations) repo->save(loc);

    NearbyServicesFinder finder(repo);

    // 5.1 Invalid radius
    auto invRad = finder.findNearby(30.3244, 78.0418, -1.0);
    TEST_ASSERT(!invRad.isSuccess(), "Negative radius rejected");
    TEST_ASSERT(invRad.getErrorCode() == "VALIDATION_ERROR", "ErrorCode is VALIDATION_ERROR");

    // 5.2 Invalid coordinates
    auto invLat = finder.findNearby(120.0, 78.0418, 5.0);
    TEST_ASSERT(invLat.getErrorCode() == "VALIDATION_ERROR", "Latitude > 90 rejected");

    // 5.3 Clock Tower nearby hospitals within 5 km
    auto nearbyHosp = finder.findNearby(30.3244, 78.0418, 5.0, LocationType::HOSPITAL);
    TEST_ASSERT(nearbyHosp.isSuccess(), "Nearby hospital search succeeded");
    TEST_ASSERT(!nearbyHosp.getData().empty(), "Found hospitals within 5 km of Clock Tower");

    // 5.4 Ascending distance sort assertion
    const auto& results = nearbyHosp.getData();
    for (size_t i = 1; i < results.size(); ++i) {
        TEST_ASSERT(results[i-1].distanceKm <= results[i].distanceKm, "Nearby results strictly sorted ascending by distance");
    }

    // 5.5 Remote coordinates producing NO_RESULTS
    auto remote = finder.findNearby(0.0, 0.0, 5.0);
    TEST_ASSERT(remote.getErrorCode() == "NO_RESULTS", "Remote coordinates return NO_RESULTS");
}

int main() {
    std::cout << "\n========================================================================\n";
    std::cout << "  JEEVAN MARG -- MODULE 3 COMPREHENSIVE AUTOMATED TEST SUITE\n";
    std::cout << "========================================================================\n\n";

    testDomainModels();
    testGraphTopology();
    testPathfindingAndCrossValidation();
    testTransportEstimator();
    testNearbySearch();

    std::cout << "\n========================================================================\n";
    std::cout << "  TEST SUMMARY: " << g_testsPassed << " / " << g_testsRun << " ASSERTIONS PASSED (";
    if (g_testsPassed == g_testsRun) {
        std::cout << "100% SUCCESS - ALL TESTS GREEN!)\n";
    } else {
        std::cout << (g_testsRun - g_testsPassed) << " FAILED)\n";
    }
    std::cout << "========================================================================\n\n";

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
