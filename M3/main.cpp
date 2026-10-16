#include "model/Location.h"
#include "repository/LocationRepository.h"
#include "datasource/LocationCsvLoader.h"
#include "datasource/RoadNetworkCsvLoader.h"
#include "datasource/TransportOptionCsvLoader.h"
#include "repository/TransportOptionRepository.h"
#include "service/RoadGraphBuilder.h"
#include "service/ShortestPathFinder.h"
#include "service/NearbyServicesFinder.h"
#include "service/RouteRequestValidator.h"
#include "service/TransportEstimator.h"
#include "service/RestApiServer.h"
#include "structure/RoadGraph.h"
#include "dto/ApiResponse.h"
#include "dto/RouteResponse.h"
#include "dto/TransportEstimate.h"
#include "model/TransportMode.h"
#include "util/GeoUtil.h"

#include "httplib.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char *argv[])
{
    cout << "===============================================================\n";
    cout << "   Dehradun Ambulance Management System - Module 3 (Navigation)\n";
    cout << "   Ambulance Dispatch & Healthcare Transport Services\n";
    cout << "===============================================================\n\n";

    string locPath = "seed/locations.csv";
    ifstream checkLoc(locPath);
    if (!checkLoc.is_open())
        locPath = "m3_navigation/seed/locations.csv";

    string edgePath = "seed/road_edges.csv";
    ifstream checkEdge(edgePath);
    if (!checkEdge.is_open())
        edgePath = "m3_navigation/seed/road_edges.csv";

    string optPath = "seed/transport_options.csv";
    ifstream checkOpt(optPath);
    if (!checkOpt.is_open())
        optPath = "m3_navigation/seed/transport_options.csv";

    vector<Location> locations = LocationCsvLoader::loadFromFile(locPath);
    InMemoryLocationRepository locationRepo;
    for (const auto &loc : locations)
    {
        locationRepo.save(loc);
    }

    vector<RoadEdge> edges = RoadNetworkCsvLoader::loadFromFile(edgePath);
    RoadGraph graph = RoadGraphBuilder::build(locationRepo, edges);
    ShortestPathFinder pathFinder(graph);
    NearbyServicesFinder nearbyFinder(locationRepo);

    auto options = TransportOptionCsvLoader::loadFromFile(optPath);
    InMemoryTransportOptionRepository transportRepo;
    for (const auto &opt : options)
    {
        transportRepo.save(opt);
    }
    TransportEstimator estimator(transportRepo);

    cout << "Emergency Road Graph initialized: " << graph.nodeCount()
         << " vertices, " << graph.edgeCount() << " edges.\n";
    cout << "Ambulance Network Locations: " << locations.size() << " loaded from " << locPath << ".\n\n";

    cout << "===============================================================\n";
    cout << "   TASK 5: NEARBY SERVICES FINDER DEMONSTRATIONS\n";
    cout << "===============================================================\n";

    auto demoNearby = [&](const string &label,
                          double lat, double lon, double radiusKm,
                          const vector<LocationType> &types)
    {
        cout << "\n--- " << label << " ---\n";
        cout << "  Query: (" << lat << ", " << lon << "), radius=" << radiusKm << " km";
        if (!types.empty())
        {
            cout << ", types=[";
            for (size_t i = 0; i < types.size(); ++i)
            {
                if (i)
                    cout << ", ";
                cout << Location::locationTypeToString(types[i]);
            }
            cout << "]";
        }
        else
        {
            cout << ", types=[ALL]";
        }
        cout << "\n";

        auto resp = nearbyFinder.findNearby(lat, lon, radiusKm, types);
        if (resp.success)
        {
            cout << "  Status: SUCCESS - " << resp.message << "\n";
            for (const auto &loc : resp.data)
            {
                double d = GeoUtil::haversineDistanceKm(lat, lon, loc.getLatitude(), loc.getLongitude());
                cout << fixed << setprecision(3)
                     << "    [" << loc.getLocationId() << "] " << loc.getName()
                     << " (" << Location::locationTypeToString(loc.getLocationType()) << ")"
                     << " - " << d << " km\n";
            }
        }
        else
        {
            cout << "  Status: FAILED [" << resp.errorCode << "] " << resp.message << "\n";
        }
    };

    demoNearby("a) Hospitals within 5 km of Clock Tower (LOC-001)",
               30.3244, 78.0418, 5.0, {LocationType::HOSPITAL});

    demoNearby("b) Emergency services within 3 km of Railway Station (LOC-003)",
               30.3155, 78.0350, 3.0, {LocationType::EMERGENCY_SERVICE});

    demoNearby("c) All types within 8 km of Parade Ground (LOC-013)",
               30.3258, 78.0465, 8.0, {});

    demoNearby("d) Invalid radius test (radius = 0)",
               30.3244, 78.0418, 0.0, {});
    demoNearby("d) Invalid radius test (radius = -2.5 km)",
               30.3244, 78.0418, -2.5, {});

    demoNearby("e) No results test (radius 0.001 km in remote coordinates)",
               30.0000, 78.0000, 0.001, {LocationType::HOSPITAL});

    cout << "\n===============================================================\n";
    cout << "   TASK 6: SHORTEST PATH DEMONSTRATION\n";
    cout << "===============================================================\n";

    string startId = "LOC-001";
    string endId = "LOC-011";

    cout << "Start: " << startId << "\n";
    cout << "Destination: " << endId << "\n\n";

    auto routeResult = pathFinder.findShortestRoute(startId, endId);

    if (!routeResult.success)
    {
        cout << "Route failed: " << routeResult.message << "\n";
        return 1;
    }

    const RouteResponse &route = routeResult.data;

    for (TransportMode mode : {TransportMode::AMBULANCE, TransportMode::CAR})
    {
        const auto estimate = estimator.estimateFor(mode, route);
        cout << toString(mode) << " estimate: "
             << (estimate.available ? "available" : "unavailable")
             << ", duration=" << fixed << setprecision(2)
             << estimate.estimatedDurationMinutes << " minutes"
             << ", cost=" << estimate.estimatedCost;
        if (!estimate.note.empty()) cout << ", note=" << estimate.note;
        cout << "\n";
    }

    cout << "Shortest Route: ";
    const auto &path = route.getPathNodeIds();

    for (size_t i = 0; i < path.size(); ++i)
    {
        if (i > 0)
            cout << " -> ";
        cout << path[i];
    }

    cout << "\nTotal Distance: "
         << fixed << setprecision(2)
         << route.getTotalDistanceKm() << " km\n";

    cout << "\nRoad Segments:\n";

    for (const auto &segment : route.getSegments())
    {
        cout << "  " << segment.getFromNodeId()
             << " -> " << segment.getToNodeId()
             << " | " << segment.getRoadName()
             << " | " << segment.getDistanceKm() << " km\n";
    }

    cout << "\nNodes Explored: "
         << route.getNodesExplored() << "\n";

    return 0;
}
