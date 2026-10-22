// Interactive (manual input) console for Module 3 - Navigation & Transport.
// Reuses the same classes as main.cpp and the tests; nothing is duplicated.

#include "model/Location.h"
#include "repository/LocationRepository.h"
#include "repository/TransportOptionRepository.h"
#include "datasource/LocationCsvLoader.h"
#include "datasource/RoadNetworkCsvLoader.h"
#include "datasource/TransportOptionCsvLoader.h"
#include "service/RoadGraphBuilder.h"
#include "service/ShortestPathFinder.h"
#include "service/NearbyServicesFinder.h"
#include "service/RouteRequestValidator.h"
#include "service/TransportEstimator.h"
#include "util/GeoUtil.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

static string firstExisting(const string &a, const string &b)
{
    ifstream f(a);
    return f.is_open() ? a : b;
}

// Read one line; returns false on EOF (Ctrl+Z / Ctrl+D)
static bool readLine(const string &prompt, string &out)
{
    cout << prompt << flush;
    return static_cast<bool>(getline(cin, out));
}

static bool readDouble(const string &prompt, double &value)
{
    string s;
    if (!readLine(prompt, s))
        return false;
    try
    {
        size_t pos = 0;
        value = stod(s, &pos);
        if (pos != s.size())
            throw invalid_argument("trailing");
    }
    catch (...)
    {
        cout << "  Invalid number: '" << s << "'\n";
        value = 0.0;
        return true; // let the module's own validation / error path respond
    }
    return true;
}

int main()
{
    string locPath = firstExisting("seed/locations.csv", "m3_navigation/seed/locations.csv");
    string edgePath = firstExisting("seed/road_edges.csv", "m3_navigation/seed/road_edges.csv");
    string optPath = firstExisting("seed/transport_options.csv", "m3_navigation/seed/transport_options.csv");

    InMemoryLocationRepository locationRepo;
    for (const auto &loc : LocationCsvLoader::loadFromFile(locPath))
        locationRepo.save(loc);

    RoadGraph graph = RoadGraphBuilder::build(locationRepo, RoadNetworkCsvLoader::loadFromFile(edgePath));
    ShortestPathFinder pathFinder(graph);
    NearbyServicesFinder nearbyFinder(locationRepo);

    InMemoryTransportOptionRepository transportRepo;
    for (const auto &opt : TransportOptionCsvLoader::loadFromFile(optPath))
        transportRepo.save(opt);
    TransportEstimator estimator(transportRepo);
    RouteRequestValidator validator(locationRepo);

    cout << "Module 3 - Navigation & Transport (interactive)\n";
    cout << graph.nodeCount() << " nodes, " << graph.edgeCount() << " edges loaded.\n";

    while (true)
    {
        cout << "\n==================== MENU ====================\n"
             << " 1. List all locations\n"
             << " 2. Shortest route (+ transport estimate)\n"
             << " 3. Nearby services\n"
             << " 4. Validate a route request only\n"
             << " 0. Exit\n"
             << "==============================================\n";

        string choice;
        if (!readLine("Choice: ", choice) || choice == "0")
            break;

        if (choice == "1")
        {
            for (const auto &l : locationRepo.findAll())
                cout << "  " << l.getLocationId() << "  " << left << setw(40) << l.getName()
                     << setw(18) << Location::locationTypeToString(l.getLocationType())
                     << fixed << setprecision(4) << l.getLatitude() << ", " << l.getLongitude() << "\n";
        }
        else if (choice == "2" || choice == "4")
        {
            string src, dst, mode;
            if (!readLine("Source ID (e.g. LOC-001): ", src) ||
                !readLine("Destination ID (e.g. LOC-011): ", dst) ||
                !readLine("Mode (AMBULANCE / CAR): ", mode))
                break;

            auto v = validator.validate(src, dst, mode);
            if (!v.success)
            {
                cout << "  FAILED [" << v.errorCode << "] " << v.message << "\n";
                continue;
            }
            cout << "  " << v.message << "  (" << v.data.toString() << ")\n";
            if (choice == "4")
                continue;

            auto r = pathFinder.findShortestRoute(v.data.getSourceLocationId(), v.data.getDestinationLocationId());
            if (!r.success)
            {
                cout << "  FAILED [" << r.errorCode << "] " << r.message << "\n";
                continue;
            }

            const RouteResponse &route = r.data;
            cout << "\n  Route: ";
            const auto &path = route.getPathNodeIds();
            for (size_t i = 0; i < path.size(); ++i)
                cout << (i ? " -> " : "") << path[i];
            cout << fixed << setprecision(2)
                 << "\n  Total distance: " << route.getTotalDistanceKm() << " km"
                 << "\n  Nodes explored: " << route.getNodesExplored() << "\n  Segments:\n";
            for (const auto &s : route.getSegments())
                cout << "    " << s.getFromNodeId() << " -> " << s.getToNodeId()
                     << " | " << s.getRoadName() << " | " << s.getDistanceKm() << " km\n";

            TransportEstimate e = estimator.estimateFor(v.data.getMode(), route);
            cout << "  " << toString(v.data.getMode()) << ": "
                 << (e.available ? "available" : "unavailable")
                 << ", " << e.estimatedDurationMinutes << " min, cost " << e.estimatedCost;
            if (!e.note.empty())
                cout << " (" << e.note << ")";
            cout << "\n";
        }
        else if (choice == "3")
        {
            double lat = 0, lng = 0, radius = 0;
            string typeText;
            if (!readDouble("Latitude  (e.g. 30.3244): ", lat) ||
                !readDouble("Longitude (e.g. 78.0418): ", lng) ||
                !readDouble("Radius km (e.g. 5): ", radius) ||
                !readLine("Type (HOSPITAL / EMERGENCY_SERVICE / TRANSPORT_HUB ... or blank for all): ", typeText))
                break;

            vector<LocationType> types;
            if (!typeText.empty())
            {
                try
                {
                    types.push_back(Location::stringToLocationType(typeText));
                }
                catch (const exception &)
                {
                    cout << "  Unknown type '" << typeText << "'.\n";
                    continue;
                }
            }

            auto resp = nearbyFinder.findNearby(lat, lng, radius, types);
            if (!resp.success)
            {
                cout << "  FAILED [" << resp.errorCode << "] " << resp.message << "\n";
                continue;
            }
            cout << "  " << resp.message << "\n";
            for (const auto &l : resp.data)
                cout << fixed << setprecision(3) << "    [" << l.getLocationId() << "] " << l.getName()
                     << " (" << Location::locationTypeToString(l.getLocationType()) << ") - "
                     << GeoUtil::haversineDistanceKm(lat, lng, l.getLatitude(), l.getLongitude()) << " km\n";
        }
        else
        {
            cout << "  Unknown choice.\n";
        }
    }

    cout << "Bye.\n";
    return 0;
}
