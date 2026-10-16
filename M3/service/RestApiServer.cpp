#include "service/RestApiServer.h"
#include "util/GeoUtil.h"

// cpp-httplib single-header include
#include "httplib.h"

#include <sstream>
#include <iomanip>
#include <chrono>

namespace {

std::string escapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\b') out += "\\b";
        else if (c == '\f') out += "\\f";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

std::string serializeNearbyJson(bool success,
                                const std::string& message,
                                const std::vector<Location>& locations,
                                double queryLat,
                                double queryLng,
                                const std::string& errorCode)
{
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"success\": " << (success ? "true" : "false") << ",\n";
    ss << "  \"message\": \"" << escapeJson(message) << "\",\n";
    ss << "  \"data\": [";
    for (size_t i = 0; i < locations.size(); ++i) {
        const auto& loc = locations[i];
        double dist = GeoUtil::haversineDistanceKm(queryLat, queryLng, loc.getLatitude(), loc.getLongitude());
        if (i > 0) ss << ",";
        ss << "\n    {\n";
        ss << "      \"id\": \"" << escapeJson(loc.getLocationId()) << "\",\n";
        ss << "      \"name\": \"" << escapeJson(loc.getName()) << "\",\n";
        ss << "      \"latitude\": " << std::fixed << std::setprecision(6) << loc.getLatitude() << ",\n";
        ss << "      \"longitude\": " << std::fixed << std::setprecision(6) << loc.getLongitude() << ",\n";
        ss << "      \"locationType\": \"" << Location::locationTypeToString(loc.getLocationType()) << "\",\n";
        ss << "      \"distance_km\": " << std::fixed << std::setprecision(4) << dist << "\n";
        ss << "    }";
    }
    if (!locations.empty()) ss << "\n  ";
    ss << "],\n";
    ss << "  \"errorCode\": \"" << escapeJson(errorCode) << "\"\n";
    ss << "}";
    return ss.str();
}

std::string serializeRouteSuccessJson(const RouteResponse& route, const std::string& message) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"success\": true,\n";
    ss << "  \"message\": \"" << escapeJson(message) << "\",\n";
    ss << "  \"data\": {\n";
    ss << "    \"startNodeId\": \"" << escapeJson(route.getStartNodeId()) << "\",\n";
    ss << "    \"endNodeId\": \"" << escapeJson(route.getEndNodeId()) << "\",\n";
    ss << "    \"totalDistanceKm\": " << std::fixed << std::setprecision(4) << route.getTotalDistanceKm() << ",\n";
    
    // pathNodeIds
    ss << "    \"pathNodeIds\": [";
    const auto& nodes = route.getPathNodeIds();
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\"" << escapeJson(nodes[i]) << "\"";
    }
    ss << "],\n";

    // segments
    ss << "    \"segments\": [";
    const auto& segs = route.getSegments();
    for (size_t i = 0; i < segs.size(); ++i) {
        const auto& s = segs[i];
        if (i > 0) ss << ",";
        ss << "\n      {\n";
        ss << "        \"fromNodeId\": \"" << escapeJson(s.getFromNodeId()) << "\",\n";
        ss << "        \"toNodeId\": \"" << escapeJson(s.getToNodeId()) << "\",\n";
        ss << "        \"roadName\": \"" << escapeJson(s.getRoadName()) << "\",\n";
        ss << "        \"distanceKm\": " << std::fixed << std::setprecision(4) << s.getDistanceKm() << ",\n";
        ss << "        \"roadType\": \"" << RoadEdge::roadTypeToString(s.getRoadType()) << "\"\n";
        ss << "      }";
    }
    if (!segs.empty()) ss << "\n    ";
    ss << "],\n";

    ss << "    \"nodesExplored\": " << route.getNodesExplored() << "\n";
    ss << "  },\n";
    ss << "  \"errorCode\": \"\"\n";
    ss << "}";
    return ss.str();
}

std::string serializeRouteFailJson(const std::string& message, const std::string& errorCode) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"success\": false,\n";
    ss << "  \"message\": \"" << escapeJson(message) << "\",\n";
    ss << "  \"data\": null,\n";
    ss << "  \"errorCode\": \"" << escapeJson(errorCode) << "\"\n";
    ss << "}";
    return ss.str();
}

std::string serializeHospitalsJson(bool success,
                                   const std::string& message,
                                   const std::vector<Location>& hospitals,
                                   const std::string& errorCode)
{
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"success\": " << (success ? "true" : "false") << ",\n";
    ss << "  \"message\": \"" << escapeJson(message) << "\",\n";
    ss << "  \"data\": [";
    for (size_t i = 0; i < hospitals.size(); ++i) {
        const auto& loc = hospitals[i];
        if (i > 0) ss << ",";
        ss << "\n    {\n";
        ss << "      \"id\": \"" << escapeJson(loc.getLocationId()) << "\",\n";
        ss << "      \"name\": \"" << escapeJson(loc.getName()) << "\",\n";
        ss << "      \"latitude\": " << std::fixed << std::setprecision(6) << loc.getLatitude() << ",\n";
        ss << "      \"longitude\": " << std::fixed << std::setprecision(6) << loc.getLongitude() << ",\n";
        ss << "      \"locationType\": \"" << Location::locationTypeToString(loc.getLocationType()) << "\"\n";
        ss << "    }";
    }
    if (!hospitals.empty()) ss << "\n  ";
    ss << "],\n";
    ss << "  \"errorCode\": \"" << escapeJson(errorCode) << "\"\n";
    ss << "}";
    return ss.str();
}

} // anonymous namespace

RestApiServer::RestApiServer(int port,
                             const ShortestPathFinder& pathFinder,
                             const NearbyServicesFinder& nearbyFinder,
                             const ILocationRepository& repo)
    : port(port > 0 ? port : 8080),
      pathFinder(pathFinder),
      nearbyFinder(nearbyFinder),
      repo(repo),
      server(std::make_unique<httplib::Server>()) {}

RestApiServer::~RestApiServer() {
    stop();
}

void RestApiServer::setupRoutes() {
    if (!server) return;

    // ENDPOINT 1: GET /nearby
    // Query params: lat (double), lng (double), radius (double), type (string, optional)
    server->Get("/nearby", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");

        if (!req.has_param("lat") || !req.has_param("lng")) {
            res.body = serializeNearbyJson(false, "Missing lat or lng parameter.", {}, 0.0, 0.0, "INVALID_ID");
            return;
        }

        if (!req.has_param("radius")) {
            res.body = serializeNearbyJson(false, "Missing radius parameter.", {}, 0.0, 0.0, "VALIDATION_ERROR");
            return;
        }

        double lat = 0.0;
        double lng = 0.0;
        double radius = 0.0;

        try {
            lat = std::stod(req.get_param_value("lat"));
            lng = std::stod(req.get_param_value("lng"));
        } catch (...) {
            res.body = serializeNearbyJson(false, "Invalid latitude or longitude format.", {}, 0.0, 0.0, "INVALID_ID");
            return;
        }

        try {
            radius = std::stod(req.get_param_value("radius"));
        } catch (...) {
            res.body = serializeNearbyJson(false, "Invalid radius format.", {}, lat, lng, "VALIDATION_ERROR");
            return;
        }

        std::vector<LocationType> filterTypes;
        if (req.has_param("type")) {
            std::string t = req.get_param_value("type");
            if (!t.empty()) {
                if (t == "HOSPITAL") {
                    filterTypes.push_back(LocationType::HOSPITAL);
                } else if (t == "EMERGENCY_SERVICE") {
                    filterTypes.push_back(LocationType::EMERGENCY_SERVICE);
                } else if (t == "TRANSPORT_HUB") {
                    filterTypes.push_back(LocationType::TRANSPORT_HUB);
                } else {
                    try {
                        filterTypes.push_back(Location::stringToLocationType(t));
                    } catch (...) {
                        res.body = serializeNearbyJson(false, "Unsupported location type: " + t, {}, lat, lng, "VALIDATION_ERROR");
                        return;
                    }
                }
            }
        }

        auto apiResp = nearbyFinder.findNearby(lat, lng, radius, filterTypes);
        if (!apiResp.success) {
            res.body = serializeNearbyJson(false, apiResp.message, {}, lat, lng, apiResp.errorCode);
        } else {
            res.body = serializeNearbyJson(true, apiResp.message, apiResp.data, lat, lng, "");
        }
    });

    // ENDPOINT 2: GET /route
    // Query params: from (string, location id), to (string, location id)
    server->Get("/route", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");

        if (!req.has_param("from") || !req.has_param("to")) {
            res.body = serializeRouteFailJson("Missing 'from' or 'to' parameter.", "INVALID_ID");
            return;
        }

        std::string from = req.get_param_value("from");
        std::string to = req.get_param_value("to");

        if (from.empty() || to.empty()) {
            res.body = serializeRouteFailJson("Parameters 'from' and 'to' must not be empty.", "INVALID_ID");
            return;
        }

        auto apiResp = pathFinder.findShortestRoute(from, to);
        if (!apiResp.success) {
            res.body = serializeRouteFailJson(apiResp.message, apiResp.errorCode);
        } else {
            res.body = serializeRouteSuccessJson(apiResp.data, apiResp.message);
        }
    });

    // ENDPOINT 3: GET /hospitals
    // Query params: radius (optional, double), lat (optional, double), lng (optional, double)
    server->Get("/hospitals", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");

        bool hasLat = req.has_param("lat");
        bool hasLng = req.has_param("lng");
        bool hasRadius = req.has_param("radius");

        // If any proximity query parameter is provided, require all three
        if (hasLat || hasLng || hasRadius) {
            if (!hasLat || !hasLng || !hasRadius) {
                res.body = serializeHospitalsJson(false,
                    "When filtering by proximity, 'lat', 'lng', and 'radius' must all be provided.",
                    {}, "VALIDATION_ERROR");
                return;
            }

            double lat = 0.0;
            double lng = 0.0;
            double radius = 0.0;

            try {
                lat = std::stod(req.get_param_value("lat"));
                lng = std::stod(req.get_param_value("lng"));
            } catch (...) {
                res.body = serializeHospitalsJson(false,
                    "Invalid latitude or longitude format.", {}, "INVALID_ID");
                return;
            }

            try {
                radius = std::stod(req.get_param_value("radius"));
            } catch (...) {
                res.body = serializeHospitalsJson(false,
                    "Invalid radius format.", {}, "VALIDATION_ERROR");
                return;
            }

            auto apiResp = nearbyFinder.findNearby(lat, lng, radius, {LocationType::HOSPITAL});
            if (!apiResp.success) {
                res.body = serializeHospitalsJson(false, apiResp.message, {}, apiResp.errorCode);
            } else {
                res.body = serializeHospitalsJson(true, apiResp.message, apiResp.data, "");
            }
            return;
        }

        // Return all hospitals from repository
        std::vector<Location> allLocs = repo.findAll();
        std::vector<Location> hospitals;
        for (const auto& loc : allLocs) {
            if (loc.getLocationType() == LocationType::HOSPITAL) {
                hospitals.push_back(loc);
            }
        }

        if (hospitals.empty()) {
            res.body = serializeHospitalsJson(false, "No hospitals found in repository.", {}, "NO_RESULTS");
        } else {
            res.body = serializeHospitalsJson(true,
                "Retrieved " + std::to_string(hospitals.size()) + " hospital(s).",
                hospitals, "");
        }
    });
}

void RestApiServer::start() {
    if (isRunning.load()) return;

    setupRoutes();
    isRunning.store(true);

    serverThread = std::make_unique<std::thread>([this]() {
        server->listen("0.0.0.0", port);
        isRunning.store(false);
    });

    // Allow socket binding to establish
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

void RestApiServer::stop() {
    if (server) {
        server->stop();
    }
    if (serverThread && serverThread->joinable()) {
        serverThread->join();
    }
    isRunning.store(false);
}
