#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include "model/Location.h"
#include "structure/RoadGraph.h"
#include "repository/LocationRepository.h"
#include "datasource/LocationCsvLoader.h"
#include "datasource/RoadNetworkCsvLoader.h"
#include "service/RoadGraphBuilder.h"
#include "service/ShortestPathFinder.h"
#include "service/NearbyServicesFinder.h"
#include "service/TransportEstimator.h"
#include "service/RestApiServer.h"

#include "persistence/Database.h"
#include "persistence/DatabaseBootstrap.h"
#include "persistence/SqliteLocationRepository.h"
#include "persistence/SqliteRoadRepository.h"
#include "persistence/SqliteHospitalRepository.h"
#include "persistence/SqliteAmbulanceRepository.h"
#include "persistence/SqliteEmergencyRepository.h"
#include "persistence/SqlitePatientLocationRepository.h"
#include "persistence/SqliteUserRepository.h"
#include "persistence/SqliteSessionRepository.h"
#include "service/EmergencyDispatcher.h"
#include "service/AuthService.h"

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

static std::string locateSeedDir() {
    std::vector<std::string> searchPaths = {
        "seed",
        "m3_navigation/seed",
        "../seed",
        "../../seed",
        "."
    };
    for (const auto& p : searchPaths) {
        if (fs::exists(p + "/locations.csv")) return p;
    }
    return "seed";
}

void printBanner() {
    std::cout << "\n";
    std::cout << "========================================================================\n";
    std::cout << "  JEEVAN MARG -- SMART EMERGENCY RESPONSE & ASSISTANCE SYSTEM\n";
    std::cout << "  Module 3 (M3): Navigation & Emergency Routing Engine (SQLite Layer)\n";
    std::cout << "  Target: Dehradun, Uttarakhand, India  |  B.Tech PBL GEU\n";
    std::cout << "========================================================================\n\n";
}

void runDemonstrationSuite(
    std::shared_ptr<ILocationRepository> /*repo*/,
    std::shared_ptr<RoadGraph> /*graph*/,
    std::shared_ptr<ShortestPathFinder> pathFinder,
    std::shared_ptr<NearbyServicesFinder> nearbyFinder,
    std::shared_ptr<TransportEstimator> estimator) {

    std::cout << "\n>>> EXECUTING MANDATORY PHASE 2 STANDALONE DEMONSTRATIONS (Photo Match 5) <<<\n\n";

    // Demo (a): Hospitals within 5 km of Clock Tower (LOC-001)
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "[Demo (a)]: Hospitals within 5.0 km of Clock Tower (LOC-001: 30.3244, 78.0418)\n";
    std::cout << "------------------------------------------------------------------------\n";
    auto demoA = nearbyFinder->findNearby(30.3244, 78.0418, 5.0, LocationType::HOSPITAL);
    if (demoA.isSuccess()) {
        std::cout << "Status: " << demoA.getStatus() << " | Count: " << demoA.getData().size() << "\n";
        for (const auto& r : demoA.getData()) {
            std::cout << "  * " << std::left << std::setw(38) << r.location.getName()
                      << " [Distance: " << std::fixed << std::setprecision(2) << r.distanceKm << " km] "
                      << " Address: " << r.location.getAddress() << "\n";
        }
    } else {
        std::cout << "Error: " << demoA.getMessage() << "\n";
    }

    // Demo (b): Emergency services within 3 km of Railway Station (LOC-003)
    std::cout << "\n------------------------------------------------------------------------\n";
    std::cout << "[Demo (b)]: Emergency Services within 3.0 km of Railway Station (LOC-003: 30.3155, 78.0331)\n";
    std::cout << "------------------------------------------------------------------------\n";
    auto demoB = nearbyFinder->findNearby(30.3155, 78.0331, 3.0, LocationType::EMERGENCY_SERVICE);
    if (demoB.isSuccess()) {
        std::cout << "Status: " << demoB.getStatus() << " | Count: " << demoB.getData().size() << "\n";
        for (const auto& r : demoB.getData()) {
            std::cout << "  * " << std::left << std::setw(32) << r.location.getName()
                      << " [Distance: " << std::fixed << std::setprecision(2) << r.distanceKm << " km]\n";
        }
    } else {
        std::cout << "Result: " << demoB.getMessage() << "\n";
    }

    // Demo (c): All facilities within 8 km of Parade Ground (LOC-013)
    std::cout << "\n------------------------------------------------------------------------\n";
    std::cout << "[Demo (c)]: All Facilities within 8.0 km of Parade Ground (LOC-013: 30.3235, 78.0460)\n";
    std::cout << "------------------------------------------------------------------------\n";
    auto demoC = nearbyFinder->findNearby(30.3235, 78.0460, 8.0, LocationType::UNKNOWN);
    if (demoC.isSuccess()) {
        std::cout << "Status: " << demoC.getStatus() << " | Count: " << demoC.getData().size() << "\n";
        for (size_t i = 0; i < std::min<size_t>(6, demoC.getData().size()); ++i) {
            const auto& r = demoC.getData()[i];
            std::cout << "  * [" << r.location.getTypeString() << "] "
                      << std::left << std::setw(34) << r.location.getName()
                      << " [Distance: " << std::fixed << std::setprecision(2) << r.distanceKm << " km]\n";
        }
    } else {
        std::cout << "Error: " << demoC.getMessage() << "\n";
    }

    // Dijkstra Pathfinding Demonstration 1: Clock Tower to ISBT Dehradun
    std::cout << "\n------------------------------------------------------------------------\n";
    std::cout << "[Routing Demo 1]: Shortest Path from Clock Tower (LOC-001) to ISBT (LOC-002)\n";
    std::cout << "------------------------------------------------------------------------\n";
    auto route1 = pathFinder->findShortestRoute("LOC-001", "LOC-002");
    if (route1.isSuccess()) {
        RouteResponse resp = route1.getData();
        estimator->estimateAll(resp);
        std::cout << "Algorithm: " << resp.getAlgorithm() << " | Distance: " << resp.getTotalDistanceKm() << " km"
                  << " | Nodes Explored: " << resp.getNodesExplored() << "\n";
        std::cout << "Path Sequence: ";
        for (size_t i = 0; i < resp.getPathNodeIds().size(); ++i) {
            std::cout << resp.getPathNodeIds()[i] << (i + 1 < resp.getPathNodeIds().size() ? " -> " : "\n");
        }
        std::cout << "Turn-by-turn Directions:\n";
        for (const auto& seg : resp.getSegments()) {
            std::cout << "  - " << seg.fromNodeId << " -> " << seg.toNodeId << " via " << seg.roadName
                      << " (" << roadTypeToString(seg.roadType) << ", " << seg.distanceKm << " km)\n";
        }
        std::cout << "Multi-Modal Travel Time & Cost:\n";
        for (const auto& [mode, minutes] : resp.getTravelTimes()) {
            if (minutes > 0) {
                std::cout << "  * " << std::left << std::setw(22) << mode << ": "
                          << std::fixed << std::setprecision(1) << minutes << " mins";
                auto costIt = resp.getEstimatedCosts().find(mode);
                if (costIt != resp.getEstimatedCosts().end()) {
                    std::cout << " | Est. Cost: Rs. " << std::fixed << std::setprecision(2) << costIt->second;
                }
                std::cout << "\n";
            }
        }
    }

    // Dijkstra Pathfinding Demonstration 2: Clock Tower to Max Super Speciality Hospital
    std::cout << "\n------------------------------------------------------------------------\n";
    std::cout << "[Routing Demo 2]: Shortest Path from Clock Tower (LOC-001) to Max Hospital (LOC-004)\n";
    std::cout << "------------------------------------------------------------------------\n";
    auto route2 = pathFinder->findShortestRoute("LOC-001", "LOC-004");
    if (route2.isSuccess()) {
        RouteResponse resp = route2.getData();
        estimator->estimateAll(resp);
        std::cout << "Algorithm: " << resp.getAlgorithm() << " | Distance: " << resp.getTotalDistanceKm() << " km\n";
        std::cout << "Path: ";
        for (size_t i = 0; i < resp.getPathNodeIds().size(); ++i) {
            std::cout << resp.getPathNodeIds()[i] << (i + 1 < resp.getPathNodeIds().size() ? " -> " : "\n");
        }
        std::cout << "Multi-Modal Travel Time:\n";
        for (const auto& [mode, minutes] : resp.getTravelTimes()) {
            if (minutes > 0) {
                std::cout << "  * " << std::left << std::setw(22) << mode << ": "
                          << std::fixed << std::setprecision(1) << minutes << " mins\n";
            }
        }
    }

    std::cout << "\n========================================================================\n";
    std::cout << " ALL STANDALONE DEMONSTRATIONS COMPLETED SUCCESSFULLY!\n";
    std::cout << "========================================================================\n\n";
}

int main(int argc, char* argv[]) {
    printBanner();

    bool serveDaemon = false;
    int port = 8080;
    std::string dbPath = "";
    bool reseed = false;
    bool seedDemo = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--serve" || arg == "-d" || arg == "--daemon") {
            serveDaemon = true;
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--db" && i + 1 < argc) {
            dbPath = argv[++i];
        } else if (arg == "--reseed") {
            reseed = true;
        } else if (arg == "--seed-demo") {
            seedDemo = true;
        }
    }

    // Determine database path
    if (dbPath.empty()) {
        std::vector<std::string> dbDirs = {"./data", "./m3_navigation/data", "../data"};
        for (const auto& d : dbDirs) {
            if (fs::exists(d) || fs::create_directories(d)) {
                dbPath = d + "/jeevanmarg.db";
                break;
            }
        }
        if (dbPath.empty()) {
            dbPath = "jeevanmarg.db";
        }
    }

    std::string seedDir = locateSeedDir();

    // 1. Initialize SQLite Database & Bootstrap
    std::cout << "[1] Initializing SQLite database at: " << dbPath << "\n";
    auto db = std::make_shared<Database>();
    db->open(dbPath);

    if (reseed) {
        std::cout << "    [--reseed] Clearing existing tables to force fresh seed...\n";
        try {
            db->exec("PRAGMA foreign_keys = OFF");
            db->exec("DELETE FROM sessions");
            db->exec("DELETE FROM users");
            db->exec("DELETE FROM emergency_status_history");
            db->exec("DELETE FROM emergencies");
            db->exec("DELETE FROM ambulance_locations");
            db->exec("DELETE FROM ambulances");
            db->exec("DELETE FROM patient_locations");
            db->exec("DELETE FROM hospital_specialties");
            db->exec("DELETE FROM hospital_beds");
            db->exec("DELETE FROM hospitals");
            db->exec("DELETE FROM roads");
            db->exec("DELETE FROM transport_options");
            db->exec("DELETE FROM locations");
            db->exec("PRAGMA foreign_keys = ON");
            db->exec("PRAGMA user_version = 0");
        } catch (const std::exception& e) {
            std::cerr << "    Reseed reset warning: " << e.what() << "\n";
        }
    }

    DatabaseBootstrap::initializeAndSeed(*db, dbPath, seedDir);
    if (seedDemo) {
        DatabaseBootstrap::seedDemoUsers(*db);
    }
    std::cout << "    -> SQLite bootstrap and seed complete.\n";

    // 2. Initialize Repositories
    std::cout << "\n[2] Initializing SQLite repositories & graph topology...\n";
    auto sqliteLocRepo = std::make_shared<SqliteLocationRepository>(*db);
    auto sqliteRoadRepo = std::make_shared<SqliteRoadRepository>(*db);
    auto sqliteHospRepo = std::make_shared<SqliteHospitalRepository>(*db);
    auto sqliteAmbRepo = std::make_shared<SqliteAmbulanceRepository>(*db);
    auto sqliteEmerRepo = std::make_shared<SqliteEmergencyRepository>(*db);
    auto sqlitePatRepo = std::make_shared<SqlitePatientLocationRepository>(*db);
    auto sqliteUserRepo = std::make_shared<SqliteUserRepository>(*db);
    auto sqliteSessionRepo = std::make_shared<SqliteSessionRepository>(*db);
    auto authService = std::make_shared<AuthService>(sqliteUserRepo, sqliteSessionRepo, sqliteAmbRepo, sqliteHospRepo);

    // ILocationRepository interface pointer for graph builder & nearby finder
    std::shared_ptr<ILocationRepository> repo = sqliteLocRepo;

    auto edges = sqliteRoadRepo->findAll();
    std::cout << "    -> Loaded " << sqliteLocRepo->count() << " locations from SQLite.\n";
    std::cout << "    -> Loaded " << edges.size() << " bidirectional road edges from SQLite.\n";

    auto graph = RoadGraphBuilder::buildGraph(*repo, edges, true);
    std::cout << "    -> Graph constructed with " << graph->getNodeCount() << " vertices and "
              << graph->getEdgeCount() << " directed edges.\n";
    std::cout << "    -> Graph fully connected? " << (graph->isConnected() ? "YES (All locations reachable)" : "NO") << "\n";

    // 3. Engine Initialization
    auto pathFinder = std::make_shared<ShortestPathFinder>(graph);
    auto nearbyFinder = std::make_shared<NearbyServicesFinder>(repo);
    auto estimator = std::make_shared<TransportEstimator>();
    auto dispatcher = std::make_shared<EmergencyDispatcher>(
        *db, sqliteEmerRepo, sqliteAmbRepo, sqliteHospRepo, graph, pathFinder, estimator);

    // 4. If daemon flag requested, spin up REST API server directly
    if (serveDaemon) {
        RestApiServer server(port, pathFinder, nearbyFinder, estimator, repo, graph,
                             sqliteHospRepo, sqliteAmbRepo, sqliteEmerRepo, sqlitePatRepo,
                             dispatcher, authService, sqliteUserRepo, "0.0.0.0", db);
        server.start(true); // blocking
        return 0;
    }

    // 5. Otherwise run standalone demonstration suite
    runDemonstrationSuite(repo, graph, pathFinder, nearbyFinder, estimator);

    // 6. Interactive CLI menu
    std::cout << "Options:\n";
    std::cout << "  1. Start REST API Server on port " << port << " for Frontend (M4)\n";
    std::cout << "  2. Query Shortest Route between any two Location IDs\n";
    std::cout << "  3. Query Nearby Facilities from GPS Coordinates\n";
    std::cout << "  4. Exit\n";
    std::cout << "\nEnter choice [1-4] (default 1): ";

    std::string input;
    if (std::getline(std::cin, input) && (input.empty() || input == "1")) {
        RestApiServer server(port, pathFinder, nearbyFinder, estimator, repo, graph,
                             sqliteHospRepo, sqliteAmbRepo, sqliteEmerRepo, sqlitePatRepo,
                             dispatcher, authService, sqliteUserRepo, "0.0.0.0", db);
        server.start(true);
    } else if (input == "2") {
        std::cout << "Enter Source Location ID (e.g., LOC-001): ";
        std::string src; std::cin >> src;
        std::cout << "Enter Destination Location ID (e.g., LOC-004): ";
        std::string dst; std::cin >> dst;
        auto res = pathFinder->findShortestRoute(src, dst);
        if (res.isSuccess()) {
            std::cout << "Distance: " << res.getData().getTotalDistanceKm() << " km\n";
            std::cout << "Route JSON: " << res.getData().toJson() << "\n";
        } else {
            std::cout << "Routing error: " << res.getMessage() << "\n";
        }
    } else if (input == "3") {
        std::cout << "Enter Latitude (e.g., 30.3244): ";
        double lat; std::cin >> lat;
        std::cout << "Enter Longitude (e.g., 78.0418): ";
        double lon; std::cin >> lon;
        std::cout << "Enter Radius in km (e.g., 5.0): ";
        double rad; std::cin >> rad;
        auto res = nearbyFinder->findNearby(lat, lon, rad);
        std::cout << NearbyServicesFinder::resultsToJson(res.getData()) << "\n";
    }

    return 0;
}
