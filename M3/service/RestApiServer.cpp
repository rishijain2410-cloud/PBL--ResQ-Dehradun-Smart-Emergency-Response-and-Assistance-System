#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include "RestApiServer.h"
#include "../include/httplib.h"
#include "../util/JsonEscape.h"
#include "../util/CryptoUtil.h"
#include "../persistence/Database.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <atomic>
#include <deque>
#include <chrono>
#include <cmath>
#include <mutex>
#include <unordered_map>

inline void setCorsHeaders(const httplib::Request& req, httplib::Response& res) {
    std::string origin = req.get_header_value("Origin");
    if (origin == "http://localhost:8080" || origin == "http://127.0.0.1:8080" ||
        origin == "http://localhost:8089" || origin == "http://127.0.0.1:8089") {
        res.set_header("Access-Control-Allow-Origin", origin);
    } else if (origin.empty()) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:8080");
    } else {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:8080");
    }
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With, X-Tracking-Token");
}

inline void setCorsHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "http://localhost:8080");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With, X-Tracking-Token");
}

struct RateLimitEntry {
    std::deque<std::chrono::steady_clock::time_point> timestamps;
};

class SosRateLimiter {
private:
    std::mutex mtx;
    std::unordered_map<std::string, RateLimitEntry> anonEntries;
    std::unordered_map<int64_t, RateLimitEntry> userEntries;
    int anonLimit = 5;
    int userLimit = 20;
    std::chrono::seconds windowSeconds{600}; // 10 minutes

public:
    SosRateLimiter() {
        const char* envLim = std::getenv("JEEVAN_SOS_LIMIT");
        if (envLim) {
            try {
                int l = std::stoi(envLim);
                if (l > 0) anonLimit = l;
            } catch (...) {}
        }
    }

    bool checkAndRecord(const std::string& clientIp, int64_t userId) {
        std::lock_guard<std::mutex> lock(mtx);
        auto now = std::chrono::steady_clock::now();
        auto cutoff = now - windowSeconds;

        if (userId > 0) {
            auto& entry = userEntries[userId];
            while (!entry.timestamps.empty() && entry.timestamps.front() < cutoff) {
                entry.timestamps.pop_front();
            }
            if (static_cast<int>(entry.timestamps.size()) >= userLimit) {
                return false;
            }
            entry.timestamps.push_back(now);
            return true;
        } else {
            std::string ipKey = clientIp.empty() ? "anonymous" : clientIp;
            auto& entry = anonEntries[ipKey];
            while (!entry.timestamps.empty() && entry.timestamps.front() < cutoff) {
                entry.timestamps.pop_front();
            }
            if (static_cast<int>(entry.timestamps.size()) >= anonLimit) {
                return false;
            }
            entry.timestamps.push_back(now);
            return true;
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        anonEntries.clear();
        userEntries.clear();
    }
};

static SosRateLimiter g_sosRateLimiter;

static std::string buildJsonError(const std::string& errCode, const std::string& message) {
    std::ostringstream ss;
    ss << "{"
       << "\"success\":false,"
       << "\"status\":\"ERROR\","
       << "\"errorCode\":\"" << JsonEscape::escape(errCode) << "\","
       << "\"message\":\"" << JsonEscape::escape(message) << "\""
       << "}";
    return ss.str();
}

static std::string hospitalToJson(const HospitalRecord& h) {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":\"" << JsonEscape::escape(h.hospitalId) << "\","
       << "\"name\":\"" << JsonEscape::escape(h.name) << "\","
       << "\"address\":\"" << JsonEscape::escape(h.address) << "\","
       << "\"phone\":\"" << JsonEscape::escape(h.phone) << "\","
       << "\"emergency\":\"" << JsonEscape::escape(h.emergency) << "\","
       << "\"totalBeds\":" << h.totalBeds << ","
       << "\"icuBeds\":" << h.icuBeds << ","
       << "\"availableIcu\":" << h.availableIcu << ","
       << "\"oxygenBeds\":" << h.oxygenBeds << ","
       << "\"specialties\":[";
    for (size_t j = 0; j < h.specialties.size(); ++j) {
        ss << "\"" << JsonEscape::escape(h.specialties[j]) << "\"";
        if (j + 1 < h.specialties.size()) ss << ",";
    }
    ss << "],"
       << "\"latitude\":" << h.latitude << ","
       << "\"longitude\":" << h.longitude << ","
       << "\"rating\":" << h.rating
       << "}";
    return ss.str();
}

static std::string userToJson(const UserRecord& u) {
    std::ostringstream ss;
    ss << "{"
       << "\"userId\":" << u.userId << ","
       << "\"role\":\"" << JsonEscape::escape(u.role) << "\","
       << "\"username\":\"" << JsonEscape::escape(u.username) << "\","
       << "\"displayName\":\"" << JsonEscape::escape(u.displayName) << "\","
       << "\"phone\":\"" << JsonEscape::escape(u.phone) << "\","
       << "\"linkedAmbulanceId\":\"" << JsonEscape::escape(u.linkedAmbulanceId) << "\","
       << "\"linkedHospitalId\":\"" << JsonEscape::escape(u.linkedHospitalId) << "\","
       << "\"isActive\":" << (u.isActive ? "true" : "false") << ","
       << "\"createdAt\":\"" << JsonEscape::escape(u.createdAt) << "\""
       << "}";
    return ss.str();
}

static std::string authUserToJson(const AuthUser& u) {
    std::ostringstream ss;
    ss << "{"
       << "\"userId\":" << u.userId << ","
       << "\"role\":\"" << JsonEscape::escape(u.role) << "\","
       << "\"username\":\"" << JsonEscape::escape(u.username) << "\","
       << "\"displayName\":\"" << JsonEscape::escape(u.displayName) << "\","
       << "\"phone\":\"" << JsonEscape::escape(u.phone) << "\","
       << "\"linkedAmbulanceId\":\"" << JsonEscape::escape(u.linkedAmbulanceId) << "\","
       << "\"linkedHospitalId\":\"" << JsonEscape::escape(u.linkedHospitalId) << "\","
       << "\"isActive\":" << (u.isActive ? "true" : "false")
       << "}";
    return ss.str();
}

static std::string emergencyToJson(const EmergencyRecord& e) {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":\"" << JsonEscape::escape(e.emergencyId) << "\","
       << "\"emergencyId\":\"" << JsonEscape::escape(e.emergencyId) << "\","
       << "\"userId\":" << e.userId << ","
       << "\"name\":\"" << JsonEscape::escape(e.callerName) << "\","
       << "\"callerName\":\"" << JsonEscape::escape(e.callerName) << "\","
       << "\"phone\":\"" << JsonEscape::escape(e.callerPhone) << "\","
       << "\"callerPhone\":\"" << JsonEscape::escape(e.callerPhone) << "\","
       << "\"location\":\"" << JsonEscape::escape(e.pickupLocation) << "\","
       << "\"pickupLocation\":\"" << JsonEscape::escape(e.pickupLocation) << "\","
       << "\"latitude\":" << e.latitude << ","
       << "\"longitude\":" << e.longitude << ","
       << "\"severity\":\"" << JsonEscape::escape(e.severity) << "\","
       << "\"conditionText\":\"" << JsonEscape::escape(e.conditionText) << "\","
       << "\"status\":\"" << JsonEscape::escape(e.status) << "\","
       << "\"assignedHospitalId\":\"" << JsonEscape::escape(e.assignedHospitalId) << "\","
       << "\"assignedAmbulanceId\":\"" << JsonEscape::escape(e.assignedAmbulanceId) << "\","
       << "\"timestamp\":\"" << JsonEscape::escape(e.createdAt.empty() ? "Just Now" : e.createdAt) << "\","
       << "\"createdAt\":\"" << JsonEscape::escape(e.createdAt) << "\""
       << "}";
    return ss.str();
}

RestApiServer::RestApiServer(
    int p,
    std::shared_ptr<ShortestPathFinder> pf,
    std::shared_ptr<NearbyServicesFinder> nf,
    std::shared_ptr<TransportEstimator> te,
    std::shared_ptr<ILocationRepository> lr,
    std::shared_ptr<RoadGraph> rg,
    std::shared_ptr<SqliteHospitalRepository> hr,
    std::shared_ptr<SqliteAmbulanceRepository> ar,
    std::shared_ptr<SqliteEmergencyRepository> er,
    std::shared_ptr<SqlitePatientLocationRepository> pr,
    std::shared_ptr<EmergencyDispatcher> disp,
    std::shared_ptr<AuthService> auth,
    std::shared_ptr<SqliteUserRepository> ur,
    const std::string& h,
    std::shared_ptr<Database> database)
    : port(p), host(h), pathFinder(pf), nearbyFinder(nf),
      transportEstimator(te), locationRepo(lr), graph(rg),
      hospitalRepo(hr), ambulanceRepo(ar), emergencyRepo(er), patientRepo(pr),
      dispatcher(disp), authService(auth), userRepo(ur), db(database),
      running(false) {
    server = std::make_unique<httplib::Server>();
    registerRoutes();
}

void RestApiServer::clearRateLimits() {
    g_sosRateLimiter.clear();
}

void RestApiServer::resetDemoData() {
    if (db) {
        try {
            Database::Transaction tx(*db);
            // 1. Unassign ambulances first so FK constraints to emergencies don't fail
            db->exec("UPDATE ambulances SET status = 'READY', assigned_emergency_id = NULL;");
            db->exec("DELETE FROM emergency_status_history;");
            db->exec("DELETE FROM emergencies;");
            db->exec("DELETE FROM patient_locations;");
            db->exec("DELETE FROM ambulance_locations;");

            // Reset all ambulances to initial READY status & station coordinates
            db->exec(
                "UPDATE ambulances SET status = 'READY', assigned_emergency_id = NULL, "
                "current_lat = 30.3244, current_lng = 78.0418 WHERE ambulance_id = 'AMB-01'; "
                "UPDATE ambulances SET status = 'READY', assigned_emergency_id = NULL, "
                "current_lat = 30.2872, current_lng = 78.0069 WHERE ambulance_id = 'AMB-02'; "
                "UPDATE ambulances SET status = 'READY', assigned_emergency_id = NULL, "
                "current_lat = 30.3340, current_lng = 78.0180 WHERE ambulance_id = 'AMB-03'; "
                "UPDATE ambulances SET status = 'READY', assigned_emergency_id = NULL, "
                "current_lat = 30.3155, current_lng = 78.0331 WHERE ambulance_id = 'AMB-04';"
            );

            // Restore hospital bed counts to original seed values
            db->exec(
                "UPDATE hospital_beds SET total_beds=220, icu_total=28, icu_available=6, oxygen_beds=45 WHERE hospital_id='LOC-004'; "
                "UPDATE hospital_beds SET total_beds=1500, icu_total=120, icu_available=18, oxygen_beds=250 WHERE hospital_id='LOC-005'; "
                "UPDATE hospital_beds SET total_beds=180, icu_total=45, icu_available=9, oxygen_beds=60 WHERE hospital_id='LOC-006'; "
                "UPDATE hospital_beds SET total_beds=500, icu_total=40, icu_available=8, oxygen_beds=80 WHERE hospital_id='LOC-007'; "
                "UPDATE hospital_beds SET total_beds=120, icu_total=20, icu_available=4, oxygen_beds=30 WHERE hospital_id='LOC-009'; "
                "UPDATE hospital_beds SET total_beds=350, icu_total=30, icu_available=5, oxygen_beds=50 WHERE hospital_id='LOC-011';"
            );

            tx.commit();
        } catch (const std::exception& ex) {
            std::cerr << "[ERROR] resetDemoData failed: " << ex.what() << std::endl;
        }
    }
    g_sosRateLimiter.clear();
}

std::string RestApiServer::extractBearerToken(const httplib::Request& req) {
    if (req.has_header("Authorization")) {
        std::string auth = req.get_header_value("Authorization");
        if (auth.rfind("Bearer ", 0) == 0) {
            return auth.substr(7);
        }
    }
    return "";
}

std::optional<AuthUser> RestApiServer::authenticate(
    const httplib::Request& req,
    httplib::Response& res,
    const std::vector<std::string>& allowedRoles) {

    setCorsHeaders(res);

    if (!authService) {
        AuthUser dummy;
        dummy.role = "ADMIN";
        dummy.username = "system";
        return dummy;
    }

    std::string token = extractBearerToken(req);
    if (token.empty()) {
        res.status = 401;
        res.set_content(buildJsonError("UNAUTHORIZED", "Missing or invalid Authorization header. Expected 'Bearer <token>'."), "application/json");
        return std::nullopt;
    }

    auto userOpt = authService->validateToken(token);
    if (!userOpt.has_value()) {
        res.status = 401;
        res.set_content(buildJsonError("UNAUTHORIZED", "Invalid, expired, or revoked session token. Please log in again."), "application/json");
        return std::nullopt;
    }

    const auto& user = *userOpt;

    if (!user.isActive) {
        res.status = 401;
        res.set_content(buildJsonError("ACCOUNT_DISABLED", "Your account has been deactivated. Please contact an administrator."), "application/json");
        return std::nullopt;
    }

    if (!allowedRoles.empty()) {
        bool roleOk = false;
        for (const auto& r : allowedRoles) {
            if (user.role == r) {
                roleOk = true;
                break;
            }
        }
        if (!roleOk) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "Insufficient permissions for role '" + user.role + "'."), "application/json");
            return std::nullopt;
        }
    }

    return user;
}

RestApiServer::~RestApiServer() {
    stop();
}

void RestApiServer::registerRoutes() {
    // Helper lambdas for request body parsing
    auto isMalformedJson = [](const std::string& body) -> bool {
        if (body.empty()) return true;
        size_t first = body.find_first_not_of(" \t\n\r");
        size_t last = body.find_last_not_of(" \t\n\r");
        if (first == std::string::npos || last == std::string::npos) return true;
        return body[first] != '{' || body[last] != '}';
    };

    auto extractStr = [](const std::string& body, const std::string& key) -> std::string {
        size_t pos = body.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        size_t colon = body.find(":", pos);
        if (colon == std::string::npos) return "";
        size_t q1 = body.find("\"", colon);
        if (q1 == std::string::npos) return "";
        size_t q2 = q1 + 1;
        while (q2 < body.size()) {
            if (body[q2] == '\\' && q2 + 1 < body.size()) {
                q2 += 2;
                continue;
            }
            if (body[q2] == '"') break;
            q2++;
        }
        if (q2 >= body.size()) return "";
        std::string raw = body.substr(q1 + 1, q2 - q1 - 1);
        std::string unescaped = JsonEscape::unescape(raw);
        if (unescaped.size() > 200) {
            unescaped = unescaped.substr(0, 200);
        }
        return unescaped;
    };

    auto extractNum = [](const std::string& body, const std::string& key, double defVal) -> double {
        size_t pos = body.find("\"" + key + "\"");
        if (pos == std::string::npos) return defVal;
        size_t colon = body.find(":", pos);
        if (colon == std::string::npos) return defVal;
        try {
            return std::stod(body.substr(colon + 1));
        } catch (...) { return defVal; }
    };

    // 0. Global OPTIONS handler for CORS preflight
    server->Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        res.status = 200;
    });

    // 1. Health check
    server->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        res.set_content("{\"status\":\"UP\",\"system\":\"JEEVAN MARG Navigation Engine\",\"version\":\"1.0.0\"}", "application/json");
    });

    // =========================================================================
    // AUTHENTICATION & ACCESS CONTROL (PHASE 3)
    // =========================================================================

    // POST /api/auth/register (public registration strictly forced to PATIENT role)
    server->Post("/api/auth/register", [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        if (!authService) {
            res.status = 500;
            res.set_content(buildJsonError("NOT_INITIALIZED", "AuthService not initialized"), "application/json");
            return;
        }

        std::string name = extractStr(req.body, "displayName");
        if (name.empty()) name = extractStr(req.body, "name");
        std::string phone = extractStr(req.body, "phone");
        std::string username = extractStr(req.body, "username");
        std::string password = extractStr(req.body, "password");
        std::string requestedRole = extractStr(req.body, "role");

        // Hard requirement: public registration cannot select DRIVER, HOSPITAL, or ADMIN
        if (!requestedRole.empty() && requestedRole != "PATIENT") {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "Public registration is restricted to PATIENT accounts only. DRIVER, HOSPITAL, and ADMIN accounts must be provisioned by an administrator."), "application/json");
            return;
        }

        auto aRes = authService->registerPatient(name, phone, username, password);
        if (!aRes.success) {
            res.status = (aRes.errorCode == "DUPLICATE_USERNAME") ? 409 : 400;
            res.set_content(buildJsonError(aRes.errorCode, aRes.message), "application/json");
            return;
        }

        res.status = 201;
        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"CREATED\",\"message\":\"" << JsonEscape::escape(aRes.message) << "\","
           << "\"token\":\"" << JsonEscape::escape(aRes.token) << "\","
           << "\"user\":" << authUserToJson(aRes.user) << "}";
        res.set_content(ss.str(), "application/json");
    });

    // POST /api/auth/login
    server->Post("/api/auth/login", [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        if (!authService) {
            res.status = 500;
            res.set_content(buildJsonError("NOT_INITIALIZED", "AuthService not initialized"), "application/json");
            return;
        }

        std::string username = extractStr(req.body, "username");
        std::string password = extractStr(req.body, "password");

        auto aRes = authService->login(username, password);
        if (!aRes.success) {
            res.status = (aRes.errorCode == "ACCOUNT_DISABLED") ? 403 : 401;
            res.set_content(buildJsonError(aRes.errorCode, aRes.message), "application/json");
            return;
        }

        res.status = 200;
        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"message\":\"" << JsonEscape::escape(aRes.message) << "\","
           << "\"token\":\"" << JsonEscape::escape(aRes.token) << "\","
           << "\"user\":" << authUserToJson(aRes.user) << "}";
        res.set_content(ss.str(), "application/json");
    });

    // POST /api/auth/logout
    server->Post("/api/auth/logout", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::string token = extractBearerToken(req);
        if (token.empty()) {
            res.status = 401;
            res.set_content(buildJsonError("UNAUTHORIZED", "Missing Bearer token"), "application/json");
            return;
        }

        if (authService) {
            authService->logout(token);
        }

        res.set_content("{\"success\":true,\"message\":\"Logged out successfully\"}", "application/json");
    });

    // GET /api/auth/me
    server->Get("/api/auth/me", [this](const httplib::Request& req, httplib::Response& res) {
        auto userOpt = authenticate(req, res);
        if (!userOpt.has_value()) return;

        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"user\":" << authUserToJson(*userOpt) << "}";
        res.set_content(ss.str(), "application/json");
    });

    // =========================================================================
    // ADMIN ENDPOINTS
    // =========================================================================

    // GET /api/admin/users
    server->Get("/api/admin/users", [this](const httplib::Request& req, httplib::Response& res) {
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        std::vector<UserRecord> users;
        if (userRepo) {
            users = userRepo->findAll();
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"count\":" << users.size() << ",\"users\":[";
        for (size_t i = 0; i < users.size(); ++i) {
            ss << userToJson(users[i]);
            if (i + 1 < users.size()) ss << ",";
        }
        ss << "],\"data\":[";
        for (size_t i = 0; i < users.size(); ++i) {
            ss << userToJson(users[i]);
            if (i + 1 < users.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    });

    // POST /api/admin/drivers
    server->Post("/api/admin/drivers", [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        std::string username = extractStr(req.body, "username");
        std::string password = extractStr(req.body, "password");
        std::string name = extractStr(req.body, "displayName");
        if (name.empty()) name = extractStr(req.body, "name");
        std::string phone = extractStr(req.body, "phone");
        std::string ambId = extractStr(req.body, "ambulanceId");

        auto aRes = authService->createDriver(*adminOpt, username, password, name, phone, ambId);
        if (!aRes.success) {
            if (aRes.errorCode == "DUPLICATE_USERNAME" || aRes.errorCode == "AMBULANCE_ALREADY_ASSIGNED") {
                res.status = 409;
            } else if (aRes.errorCode == "RESOURCE_NOT_FOUND") {
                res.status = 404;
            } else {
                res.status = 400;
            }
            res.set_content(buildJsonError(aRes.errorCode, aRes.message), "application/json");
            return;
        }

        res.status = 201;
        std::ostringstream ss;
        ss << "{\"success\":true,\"message\":\"" << JsonEscape::escape(aRes.message) << "\",\"user\":" << authUserToJson(aRes.user) << "}";
        res.set_content(ss.str(), "application/json");
    });

    // POST /api/admin/hospitals
    server->Post("/api/admin/hospitals", [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        std::string username = extractStr(req.body, "username");
        std::string password = extractStr(req.body, "password");
        std::string name = extractStr(req.body, "displayName");
        if (name.empty()) name = extractStr(req.body, "name");
        std::string phone = extractStr(req.body, "phone");
        std::string hospId = extractStr(req.body, "hospitalId");

        auto aRes = authService->createHospital(*adminOpt, username, password, name, phone, hospId);
        if (!aRes.success) {
            if (aRes.errorCode == "DUPLICATE_USERNAME") {
                res.status = 409;
            } else if (aRes.errorCode == "RESOURCE_NOT_FOUND") {
                res.status = 404;
            } else {
                res.status = 400;
            }
            res.set_content(buildJsonError(aRes.errorCode, aRes.message), "application/json");
            return;
        }

        res.status = 201;
        std::ostringstream ss;
        ss << "{\"success\":true,\"message\":\"" << JsonEscape::escape(aRes.message) << "\",\"user\":" << authUserToJson(aRes.user) << "}";
        res.set_content(ss.str(), "application/json");
    });

    // PATCH /api/admin/users/:id/status
    server->Patch(R"(/api/admin/users/(\d+)/status)", [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        int64_t targetId = std::stoll(req.matches[1]);
        bool active = true;
        std::string statusStr = extractStr(req.body, "status");
        if (req.body.find("\"isActive\":false") != std::string::npos || statusStr == "INACTIVE" || statusStr == "DISABLED") {
            active = false;
        }

        bool ok = authService->setUserActive(*adminOpt, targetId, active);
        if (ok) {
            res.set_content("{\"success\":true,\"message\":\"User status updated successfully\"}", "application/json");
        } else {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "User ID not found"), "application/json");
        }
    });


    // POST /api/admin/users/:id/reset-password
    server->Post(R"(/api/admin/users/(\d+)/reset-password)", [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        int64_t targetId = std::stoll(req.matches[1]);
        std::string newPassword = extractStr(req.body, "newPassword");
        if (newPassword.empty()) newPassword = extractStr(req.body, "password");

        if (newPassword.size() < 8) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "New password must be at least 8 characters long"), "application/json");
            return;
        }

        bool ok = authService->resetPassword(*adminOpt, targetId, newPassword);
        if (ok) {
            res.set_content("{\"success\":true,\"message\":\"Password reset successfully\"}", "application/json");
        } else {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "User ID not found"), "application/json");
        }
    });

    // POST /api/admin/reset-demo (ADMIN only)
    server->Post("/api/admin/reset-demo", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        resetDemoData();

        std::ostringstream ss;
        ss << "{\"success\":true,\"message\":\"Demo data reset successfully.\"}";
        res.set_content(ss.str(), "application/json");
    });

    // PATCH /api/admin/ambulances/:id/status and /api/ambulances/:id/status
    auto handleAdminAmbulanceStatus = [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto userOpt = authenticate(req, res, {"ADMIN", "DRIVER"});
        if (!userOpt.has_value()) return;

        std::string ambId = req.matches[1];
        if (userOpt->role == "DRIVER" && userOpt->linkedAmbulanceId != ambId) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "Cannot modify another driver's ambulance"), "application/json");
            return;
        }

        std::string newStatus = extractStr(req.body, "status");
        if (newStatus.empty()) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Missing status field"), "application/json");
            return;
        }

        auto ambOpt = ambulanceRepo ? ambulanceRepo->findById(ambId) : std::nullopt;
        if (!ambOpt.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Ambulance not found: " + ambId), "application/json");
            return;
        }

        if (!ambOpt->assignedEmergencyId.empty() && (newStatus == "READY" || newStatus == "MAINTENANCE")) {
            res.status = 409;
            res.set_content(buildJsonError("AMBULANCE_BUSY", "Cannot change standby status while actively handling emergency " + ambOpt->assignedEmergencyId), "application/json");
            return;
        }

        bool ok = ambulanceRepo->updateStatus(ambId, newStatus, ambOpt->assignedEmergencyId);
        if (ok) {
            res.set_content("{\"success\":true,\"message\":\"Ambulance " + ambId + " status updated to " + newStatus + "\"}", "application/json");
        } else {
            res.status = 500;
            res.set_content(buildJsonError("DATABASE_ERROR", "Failed to update ambulance status"), "application/json");
        }
    };
    server->Patch(R"(/api/admin/ambulances/([^/]+)/status)", handleAdminAmbulanceStatus);
    server->Patch(R"(/api/ambulances/([^/]+)/status)", handleAdminAmbulanceStatus);

    // =========================================================================
    // DRIVER ROLE ENDPOINTS
    // =========================================================================

    // Handler for Driver's assigned ambulance
    auto handleDriverAmbulance = [this](const httplib::Request& req, httplib::Response& res) {
        auto driverOpt = authenticate(req, res, {"DRIVER", "ADMIN"});
        if (!driverOpt.has_value()) return;

        if (driverOpt->linkedAmbulanceId.empty()) {
            res.status = 404;
            res.set_content(buildJsonError("NO_LINKED_AMBULANCE", "No ambulance assigned to this driver account"), "application/json");
            return;
        }

        auto ambOpt = ambulanceRepo ? ambulanceRepo->findById(driverOpt->linkedAmbulanceId) : std::nullopt;
        if (!ambOpt.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Linked ambulance not found in system"), "application/json");
            return;
        }

        const auto& a = *ambOpt;
        std::ostringstream ss;
        ss << "{\"success\":true,"
           << "\"id\":\"" << JsonEscape::escape(a.ambulanceId) << "\","
           << "\"ambulanceId\":\"" << JsonEscape::escape(a.ambulanceId) << "\","
           << "\"unitCode\":\"" << JsonEscape::escape(a.unitCode) << "\","
           << "\"type\":\"" << JsonEscape::escape(a.type) << "\","
           << "\"driver\":\"" << JsonEscape::escape(a.driverName) << "\","
           << "\"driverName\":\"" << JsonEscape::escape(a.driverName) << "\","
           << "\"phone\":\"" << JsonEscape::escape(a.driverPhone) << "\","
           << "\"driverPhone\":\"" << JsonEscape::escape(a.driverPhone) << "\","
           << "\"stationLocationId\":\"" << JsonEscape::escape(a.stationLocationId) << "\","
           << "\"latitude\":" << a.currentLat << ","
           << "\"longitude\":" << a.currentLng << ","
           << "\"currentLat\":" << a.currentLat << ","
           << "\"currentLng\":" << a.currentLng << ","
           << "\"status\":\"" << JsonEscape::escape(a.status) << "\","
           << "\"assignedEmergencyId\":\"" << JsonEscape::escape(a.assignedEmergencyId) << "\","
           << "\"data\":{"
           << "\"id\":\"" << JsonEscape::escape(a.ambulanceId) << "\","
           << "\"ambulanceId\":\"" << JsonEscape::escape(a.ambulanceId) << "\","
           << "\"unitCode\":\"" << JsonEscape::escape(a.unitCode) << "\","
           << "\"type\":\"" << JsonEscape::escape(a.type) << "\","
           << "\"driverName\":\"" << JsonEscape::escape(a.driverName) << "\","
           << "\"driverPhone\":\"" << JsonEscape::escape(a.driverPhone) << "\","
           << "\"stationLocationId\":\"" << JsonEscape::escape(a.stationLocationId) << "\","
           << "\"currentLat\":" << a.currentLat << ","
           << "\"currentLng\":" << a.currentLng << ","
           << "\"status\":\"" << JsonEscape::escape(a.status) << "\","
           << "\"assignedEmergencyId\":\"" << JsonEscape::escape(a.assignedEmergencyId) << "\""
           << "}}";
        res.set_content(ss.str(), "application/json");
    };

    server->Get("/api/driver/my-ambulance", handleDriverAmbulance);
    server->Get("/api/driver/ambulance", handleDriverAmbulance);

    // Handler for Driver's assigned emergency
    auto handleDriverAssignedEmergency = [this](const httplib::Request& req, httplib::Response& res) {
        auto driverOpt = authenticate(req, res, {"DRIVER", "ADMIN"});
        if (!driverOpt.has_value()) return;

        if (driverOpt->linkedAmbulanceId.empty()) {
            res.status = 404;
            res.set_content(buildJsonError("NO_LINKED_AMBULANCE", "No ambulance linked to this driver"), "application/json");
            return;
        }

        auto ambOpt = ambulanceRepo ? ambulanceRepo->findById(driverOpt->linkedAmbulanceId) : std::nullopt;
        if (!ambOpt.has_value() || ambOpt->assignedEmergencyId.empty()) {
            res.set_content("{\"success\":true,\"message\":\"No active emergency assigned\",\"data\":null}", "application/json");
            return;
        }

        auto emOpt = emergencyRepo ? emergencyRepo->findById(ambOpt->assignedEmergencyId) : std::nullopt;
        if (!emOpt.has_value()) {
            res.set_content("{\"success\":true,\"message\":\"Assigned emergency not found\",\"data\":null}", "application/json");
            return;
        }

        const auto& em = *emOpt;
        std::ostringstream ss;
        ss << "{\"success\":true,\"data\":{"
           << "\"emergencyId\":\"" << JsonEscape::escape(em.emergencyId) << "\","
           << "\"patientName\":\"" << JsonEscape::escape(em.callerName) << "\","
           << "\"callerName\":\"" << JsonEscape::escape(em.callerName) << "\","
           << "\"patientPhone\":\"" << JsonEscape::escape(em.callerPhone) << "\","
           << "\"callerPhone\":\"" << JsonEscape::escape(em.callerPhone) << "\","
           << "\"pickupLocation\":\"" << JsonEscape::escape(em.pickupLocation) << "\","
           << "\"location\":\"" << JsonEscape::escape(em.pickupLocation) << "\","
           << "\"latitude\":" << em.latitude << ","
           << "\"longitude\":" << em.longitude << ","
           << "\"severity\":\"" << JsonEscape::escape(em.severity) << "\","
           << "\"triageCategory\":\"" << JsonEscape::escape(em.severity) << "\","
           << "\"condition\":\"" << JsonEscape::escape(em.conditionText) << "\","
           << "\"conditionText\":\"" << JsonEscape::escape(em.conditionText) << "\","
           << "\"status\":\"" << JsonEscape::escape(em.status) << "\","
           << "\"assignedAmbulanceId\":\"" << JsonEscape::escape(em.assignedAmbulanceId) << "\","
           << "\"assignedHospitalId\":\"" << JsonEscape::escape(em.assignedHospitalId) << "\","
           << "\"createdAt\":\"" << JsonEscape::escape(em.createdAt) << "\"";

        if (ambulanceRepo) {
            ss << ",\"ambulance\":{"
               << "\"id\":\"" << JsonEscape::escape(ambOpt->ambulanceId) << "\","
               << "\"unitCode\":\"" << JsonEscape::escape(ambOpt->unitCode) << "\","
               << "\"currentLat\":" << ambOpt->currentLat << ","
               << "\"currentLng\":" << ambOpt->currentLng << ","
               << "\"status\":\"" << JsonEscape::escape(ambOpt->status) << "\""
               << "}";
        }

        if (hospitalRepo && !em.assignedHospitalId.empty()) {
            auto hosp = hospitalRepo->findById(em.assignedHospitalId);
            if (hosp.has_value()) {
                ss << ",\"hospital\":{"
                   << "\"id\":\"" << JsonEscape::escape(hosp->hospitalId) << "\","
                   << "\"name\":\"" << JsonEscape::escape(hosp->name) << "\","
                   << "\"address\":\"" << JsonEscape::escape(hosp->address) << "\","
                   << "\"latitude\":" << hosp->latitude << ","
                   << "\"longitude\":" << hosp->longitude << ","
                   << "\"phone\":\"" << JsonEscape::escape(hosp->phone) << "\","
                   << "\"availableIcu\":" << hosp->availableIcu
                   << "}";
            }
        }
        ss << "}}";
        res.set_content(ss.str(), "application/json");
    };

    server->Get("/api/driver/assigned-emergency", handleDriverAssignedEmergency);
    server->Get("/api/driver/emergency", handleDriverAssignedEmergency);

    // POST /api/driver/location
    auto handleDriverLocation = [this, extractNum](const httplib::Request& req, httplib::Response& res) {
        auto driverOpt = authenticate(req, res, {"DRIVER", "ADMIN"});
        if (!driverOpt.has_value()) return;

        if (driverOpt->linkedAmbulanceId.empty()) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "No ambulance assigned to this driver"), "application/json");
            return;
        }

        double lat = extractNum(req.body, "latitude", 0.0);
        double lng = extractNum(req.body, "longitude", 0.0);
        if (lat < -90.0 || lat > 90.0 || lng < -180.0 || lng > 180.0 || (lat == 0.0 && lng == 0.0)) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Invalid GPS coordinates"), "application/json");
            return;
        }

        if (ambulanceRepo) {
            ambulanceRepo->updatePosition(driverOpt->linkedAmbulanceId, lat, lng);
            ambulanceRepo->appendLocation(driverOpt->linkedAmbulanceId, lat, lng);
        }

        res.set_content("{\"success\":true,\"message\":\"Ambulance location updated for " + driverOpt->linkedAmbulanceId + "\"}", "application/json");
    };

    server->Post("/api/driver/location", handleDriverLocation);

    // PATCH /api/driver/ambulance/status (Availability & Emergency Lifecycle)
    auto handleDriverAmbulanceStatus = [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        auto driverOpt = authenticate(req, res, {"DRIVER", "ADMIN"});
        if (!driverOpt.has_value()) return;

        if (driverOpt->linkedAmbulanceId.empty()) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "No ambulance linked to this driver"), "application/json");
            return;
        }

        std::string newStatus = extractStr(req.body, "status");
        if (newStatus.empty()) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Missing status field"), "application/json");
            return;
        }

        auto ambOpt = ambulanceRepo ? ambulanceRepo->findById(driverOpt->linkedAmbulanceId) : std::nullopt;
        if (!ambOpt.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Ambulance not found"), "application/json");
            return;
        }

        // Case 1: Driver has an active emergency assigned
        if (!ambOpt->assignedEmergencyId.empty()) {
            std::string emId = ambOpt->assignedEmergencyId;

            // Cannot manually set READY or MAINTENANCE while an active emergency is assigned
            if (newStatus == "READY" || newStatus == "MAINTENANCE") {
                res.status = 400;
                res.set_content(buildJsonError("INVALID_STATUS_TRANSITION",
                    "Cannot mark ambulance as " + newStatus + " while handling active emergency (" + emId + "). Advance the emergency lifecycle or complete it."),
                    "application/json");
                return;
            }

            if (dispatcher) {
                // If status is an emergency state transition
                if (newStatus == "ON_SCENE" || newStatus == "TRANSPORTING" || newStatus == "ARRIVED" || newStatus == "COMPLETED" || newStatus == "CANCELLED") {
                    auto tRes = dispatcher->transitionStatus(emId, newStatus, "Driver status update: " + newStatus);
                    if (!tRes.success) {
                        res.status = 400;
                        res.set_content(buildJsonError(tRes.errorCode, tRes.message), "application/json");
                        return;
                    }
                    res.set_content("{\"success\":true,\"message\":\"" + JsonEscape::escape(tRes.message) + "\",\"ambulanceStatus\":\"" + JsonEscape::escape(tRes.ambulanceStatus) + "\"}", "application/json");
                    return;
                } else if (newStatus == "EN_ROUTE" || newStatus == "DISPATCHED") {
                    ambulanceRepo->updateStatus(driverOpt->linkedAmbulanceId, "DISPATCHED", emId);
                    res.set_content("{\"success\":true,\"message\":\"Ambulance status is EN_ROUTE\",\"ambulanceStatus\":\"DISPATCHED\"}", "application/json");
                    return;
                }
            } else {
                if (emergencyRepo) emergencyRepo->updateStatus(emId, newStatus, "Driver status update");
                res.set_content("{\"success\":true,\"message\":\"Status updated to " + JsonEscape::escape(newStatus) + "\"}", "application/json");
                return;
            }
        } else {
            // Case 2: No emergency assigned -> can toggle READY or MAINTENANCE
            if (newStatus == "READY" || newStatus == "MAINTENANCE") {
                if (ambulanceRepo) {
                    ambulanceRepo->updateStatus(driverOpt->linkedAmbulanceId, newStatus, "");
                }
                res.set_content("{\"success\":true,\"message\":\"Ambulance availability set to " + JsonEscape::escape(newStatus) + "\",\"status\":\"" + JsonEscape::escape(newStatus) + "\"}", "application/json");
                return;
            } else {
                res.status = 400;
                res.set_content(buildJsonError("INVALID_STATUS", "Without an assigned emergency, ambulance can only be marked READY or MAINTENANCE"), "application/json");
                return;
            }
        }
    };

    server->Patch("/api/driver/ambulance/status", handleDriverAmbulanceStatus);
    server->Patch("/api/driver/emergency-status", handleDriverAmbulanceStatus);

    // =========================================================================
    // HOSPITAL ROLE ENDPOINTS
    // =========================================================================

    // GET /api/hospital/profile and /api/hospital/my-hospital
    auto handleHospitalProfile = [this](const httplib::Request& req, httplib::Response& res) {
        auto hospOpt = authenticate(req, res, {"HOSPITAL", "ADMIN"});
        if (!hospOpt.has_value()) return;

        if (hospOpt->linkedHospitalId.empty()) {
            res.status = 404;
            res.set_content(buildJsonError("NO_LINKED_HOSPITAL", "No hospital linked to this account"), "application/json");
            return;
        }

        auto hRec = hospitalRepo ? hospitalRepo->findById(hospOpt->linkedHospitalId) : std::nullopt;
        if (!hRec.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Hospital not found"), "application/json");
            return;
        }

        const auto& h = *hRec;
        std::ostringstream ss;
        ss << "{\"success\":true,"
           << "\"hospitalId\":\"" << JsonEscape::escape(h.hospitalId) << "\","
           << "\"name\":\"" << JsonEscape::escape(h.name) << "\","
           << "\"address\":\"" << JsonEscape::escape(h.address) << "\","
           << "\"latitude\":" << h.latitude << ","
           << "\"longitude\":" << h.longitude << ","
           << "\"phone\":\"" << JsonEscape::escape(h.phone) << "\","
           << "\"emergencyService\":\"" << JsonEscape::escape(h.emergency) << "\","
           << "\"rating\":" << h.rating << ","
           << "\"totalBeds\":" << h.totalBeds << ","
           << "\"icuBeds\":" << h.icuBeds << ","
           << "\"icuTotal\":" << h.icuBeds << ","
           << "\"availableIcu\":" << h.availableIcu << ","
           << "\"icuAvailable\":" << h.availableIcu << ","
           << "\"oxygenBeds\":" << h.oxygenBeds << ","
           << "\"data\":" << hospitalToJson(h)
           << "}";
        res.set_content(ss.str(), "application/json");
    };

    server->Get("/api/hospital/my-hospital", handleHospitalProfile);
    server->Get("/api/hospital/profile", handleHospitalProfile);

    // GET /api/hospital/beds
    server->Get("/api/hospital/beds", [this](const httplib::Request& req, httplib::Response& res) {
        auto hospOpt = authenticate(req, res, {"HOSPITAL", "ADMIN"});
        if (!hospOpt.has_value()) return;

        if (hospOpt->linkedHospitalId.empty()) {
            res.status = 404;
            res.set_content(buildJsonError("NO_LINKED_HOSPITAL", "No hospital linked to this account"), "application/json");
            return;
        }

        auto hRec = hospitalRepo ? hospitalRepo->findById(hospOpt->linkedHospitalId) : std::nullopt;
        if (!hRec.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Hospital not found"), "application/json");
            return;
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"data\":{"
           << "\"hospitalId\":\"" << JsonEscape::escape(hRec->hospitalId) << "\","
           << "\"hospitalName\":\"" << JsonEscape::escape(hRec->name) << "\","
           << "\"totalBeds\":" << hRec->totalBeds << ","
           << "\"icuTotal\":" << hRec->icuBeds << ","
           << "\"icuAvailable\":" << hRec->availableIcu << ","
           << "\"oxygenBeds\":" << hRec->oxygenBeds
           << "}}";
        res.set_content(ss.str(), "application/json");
    });

    // GET /api/hospital/emergencies
    server->Get("/api/hospital/emergencies", [this](const httplib::Request& req, httplib::Response& res) {
        auto hospOpt = authenticate(req, res, {"HOSPITAL", "ADMIN"});
        if (!hospOpt.has_value()) return;

        if (hospOpt->linkedHospitalId.empty()) {
            res.status = 404;
            res.set_content(buildJsonError("NO_LINKED_HOSPITAL", "No hospital linked to this account"), "application/json");
            return;
        }

        std::vector<EmergencyRecord> list;
        if (emergencyRepo) {
            list = emergencyRepo->findByHospitalId(hospOpt->linkedHospitalId);
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"count\":" << list.size() << ",\"emergencies\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            ss << emergencyToJson(list[i]);
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "],\"data\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            ss << emergencyToJson(list[i]);
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    });

    // PATCH /api/hospital/beds (updates beds with validation)
    server->Patch("/api/hospital/beds", [this, extractNum](const httplib::Request& req, httplib::Response& res) {
        auto hospOpt = authenticate(req, res, {"HOSPITAL", "ADMIN"});
        if (!hospOpt.has_value()) return;

        if (hospOpt->linkedHospitalId.empty()) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "No hospital linked to this account"), "application/json");
            return;
        }

        int totalBeds = static_cast<int>(extractNum(req.body, "totalBeds", -1));
        int icuTotal = static_cast<int>(extractNum(req.body, "icuTotal", -1));
        int icuAvailable = static_cast<int>(extractNum(req.body, "icuAvailable", -1));
        int oxygenBeds = static_cast<int>(extractNum(req.body, "oxygenBeds", -1));

        // Validate non-negative numbers if specified
        if ((totalBeds != -1 && totalBeds < 0) ||
            (icuTotal != -1 && icuTotal < 0) ||
            (icuAvailable != -1 && icuAvailable < 0) ||
            (oxygenBeds != -1 && oxygenBeds < 0)) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Bed values cannot be negative"), "application/json");
            return;
        }

        // Get current values to validate available ICU <= total ICU
        if (hospitalRepo) {
            auto cur = hospitalRepo->findById(hospOpt->linkedHospitalId);
            if (cur.has_value()) {
                int checkTotalIcu = (icuTotal >= 0) ? icuTotal : cur->icuBeds;
                int checkAvailIcu = (icuAvailable >= 0) ? icuAvailable : cur->availableIcu;
                if (checkAvailIcu > checkTotalIcu) {
                    res.status = 400;
                    res.set_content(buildJsonError("VALIDATION_ERROR", "Available ICU beds cannot exceed total ICU beds"), "application/json");
                    return;
                }
            }
            hospitalRepo->updateBeds(hospOpt->linkedHospitalId, totalBeds, icuTotal, icuAvailable, oxygenBeds);
        }

        auto updatedHosp = hospitalRepo ? hospitalRepo->findById(hospOpt->linkedHospitalId) : std::nullopt;
        std::ostringstream ss;
        ss << "{\"success\":true,\"message\":\"Beds updated for hospital " << hospOpt->linkedHospitalId << "\"";
        if (updatedHosp.has_value()) {
            ss << ",\"totalBeds\":" << updatedHosp->totalBeds
               << ",\"icuTotal\":" << updatedHosp->icuBeds
               << ",\"icuAvailable\":" << updatedHosp->availableIcu
               << ",\"oxygenBeds\":" << updatedHosp->oxygenBeds;
        }
        ss << "}";
        res.set_content(ss.str(), "application/json");
    });

    // POST /api/hospital/admit and POST /api/hospital/emergencies/:id/admit
    auto handleHospitalAdmission = [this, extractStr](const httplib::Request& req, httplib::Response& res, const std::string& pathEmId) {
        setCorsHeaders(res);
        auto hospOpt = authenticate(req, res, {"HOSPITAL", "ADMIN"});
        if (!hospOpt.has_value()) return;

        std::string emId = pathEmId.empty() ? extractStr(req.body, "emergencyId") : pathEmId;
        if (emId.empty()) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Missing emergencyId"), "application/json");
            return;
        }

        auto emOpt = emergencyRepo ? emergencyRepo->findById(emId) : std::nullopt;
        if (!emOpt.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Emergency not found: " + emId), "application/json");
            return;
        }

        if (hospOpt->role == "HOSPITAL" && emOpt->assignedHospitalId != hospOpt->linkedHospitalId) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "Emergency not routed to your hospital"), "application/json");
            return;
        }

        if (dispatcher) {
            // Safely step through intermediate states if needed so lifecycle history & telemetry are complete
            if (emOpt->status == "PENDING" || emOpt->status == "ASSIGNED" || emOpt->status == "DISPATCHED") {
                dispatcher->transitionStatus(emId, "ON_SCENE", "Expedited on-scene arrival");
                dispatcher->transitionStatus(emId, "TRANSPORTING", "Expedited hospital transport");
                dispatcher->transitionStatus(emId, "ARRIVED", "Arrived at hospital emergency entrance");
            } else if (emOpt->status == "ON_SCENE") {
                dispatcher->transitionStatus(emId, "TRANSPORTING", "Expedited hospital transport");
                dispatcher->transitionStatus(emId, "ARRIVED", "Arrived at hospital emergency entrance");
            } else if (emOpt->status == "TRANSPORTING") {
                dispatcher->transitionStatus(emId, "ARRIVED", "Hospital confirmed patient arrival");
            }
            auto cRes = dispatcher->transitionStatus(emId, "COMPLETED", "Hospital admission confirmed and completed");
            if (!cRes.success && emOpt->status != "COMPLETED") {
                if (emergencyRepo) emergencyRepo->updateStatus(emId, "COMPLETED", "Hospital admission confirmed");
                if (ambulanceRepo && !emOpt->assignedAmbulanceId.empty()) {
                    ambulanceRepo->updateStatus(emOpt->assignedAmbulanceId, "READY", "");
                }
            }
        } else {
            if (emergencyRepo) emergencyRepo->updateStatus(emId, "COMPLETED", "Hospital admission confirmed");
            if (ambulanceRepo && !emOpt->assignedAmbulanceId.empty()) {
                ambulanceRepo->updateStatus(emOpt->assignedAmbulanceId, "READY", "");
            }
        }

        res.set_content("{\"success\":true,\"message\":\"Patient admission confirmed for emergency " + emId + "\"}", "application/json");
    };

    // POST /api/hospital/accept and POST /api/hospital/emergencies/:id/accept (Hospital acknowledges & prepares ER)
    auto handleHospitalAccept = [this, extractStr](const httplib::Request& req, httplib::Response& res, const std::string& pathEmId) {
        setCorsHeaders(res);
        auto hospOpt = authenticate(req, res, {"HOSPITAL", "ADMIN"});
        if (!hospOpt.has_value()) return;

        std::string emId = pathEmId.empty() ? extractStr(req.body, "emergencyId") : pathEmId;
        if (emId.empty()) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Missing emergencyId"), "application/json");
            return;
        }

        auto emOpt = emergencyRepo ? emergencyRepo->findById(emId) : std::nullopt;
        if (!emOpt.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Emergency not found: " + emId), "application/json");
            return;
        }

        if (hospOpt->role == "HOSPITAL" && emOpt->assignedHospitalId != hospOpt->linkedHospitalId) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "Emergency not routed to your hospital"), "application/json");
            return;
        }

        if (emergencyRepo) {
            emergencyRepo->updateStatus(emId, emOpt->status, "Hospital accepted incoming dispatch and prepared emergency trauma/ICU team");
        }

        res.set_content("{\"success\":true,\"message\":\"Hospital accepted emergency " + emId + " and pre-allocated emergency resources\",\"emergencyId\":\"" + emId + "\"}", "application/json");
    };

    server->Post("/api/hospital/admit", [handleHospitalAdmission](const httplib::Request& req, httplib::Response& res) {
        handleHospitalAdmission(req, res, "");
    });
    server->Post(R"(/api/hospital/emergencies/([^/]+)/admit)", [handleHospitalAdmission](const httplib::Request& req, httplib::Response& res) {
        handleHospitalAdmission(req, res, req.matches[1]);
    });
    server->Post("/api/hospital/accept", [handleHospitalAccept](const httplib::Request& req, httplib::Response& res) {
        handleHospitalAccept(req, res, "");
    });
    server->Post(R"(/api/hospital/emergencies/([^/]+)/accept)", [handleHospitalAccept](const httplib::Request& req, httplib::Response& res) {
        handleHospitalAccept(req, res, req.matches[1]);
    });

    // PATCH /api/hospitals/:id/beds (HOSPITAL own or ADMIN only)
    server->Patch(R"(/api/hospitals/([^/]+)/beds)", [this, extractNum](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto userOpt = authenticate(req, res, {"HOSPITAL", "ADMIN"});
        if (!userOpt.has_value()) return;

        std::string targetHospId = req.matches[1];
        if (userOpt->role == "HOSPITAL" && userOpt->linkedHospitalId != targetHospId) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "Access denied: cannot modify another hospital's bed availability."), "application/json");
            return;
        }

        int totalBeds = static_cast<int>(extractNum(req.body, "totalBeds", -1));
        int icuTotal = static_cast<int>(extractNum(req.body, "icuTotal", -1));
        int icuAvailable = static_cast<int>(extractNum(req.body, "icuAvailable", -1));
        int oxygenBeds = static_cast<int>(extractNum(req.body, "oxygenBeds", -1));

        if (hospitalRepo) {
            hospitalRepo->updateBeds(targetHospId, totalBeds, icuTotal, icuAvailable, oxygenBeds);
        }

        res.set_content("{\"success\":true,\"message\":\"Beds updated for hospital " + targetHospId + "\"}", "application/json");
    });

    // =========================================================================
    // PATIENT ROLE ENDPOINTS
    // =========================================================================

    // GET /api/patient/emergencies and /api/emergencies/my-history
    auto handlePatientEmergencies = [this](const httplib::Request& req, httplib::Response& res) {
        auto patOpt = authenticate(req, res, {"PATIENT", "ADMIN"});
        if (!patOpt.has_value()) return;

        std::vector<EmergencyRecord> list;
        if (emergencyRepo) {
            if (patOpt->role == "ADMIN") {
                list = emergencyRepo->findAll();
            } else {
                list = emergencyRepo->findByUserId(patOpt->userId);
            }
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"count\":" << list.size() << ",\"emergencies\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            ss << emergencyToJson(list[i]);
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "],\"data\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            ss << emergencyToJson(list[i]);
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    };
    server->Get("/api/patient/emergencies", handlePatientEmergencies);
    server->Get("/api/emergencies/my-history", handlePatientEmergencies);

    // Helper to format full emergency JSON with attached telemetry, live Dijkstra route & ETA
    auto formatEmergencyDetailsJson = [this](const EmergencyRecord& em) -> std::string {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":\"" << JsonEscape::escape(em.emergencyId) << "\","
           << "\"emergencyId\":\"" << JsonEscape::escape(em.emergencyId) << "\","
           << "\"userId\":" << em.userId << ","
           << "\"callerName\":\"" << JsonEscape::escape(em.callerName) << "\","
           << "\"patientName\":\"" << JsonEscape::escape(em.callerName) << "\","
           << "\"callerPhone\":\"" << JsonEscape::escape(em.callerPhone) << "\","
           << "\"patientPhone\":\"" << JsonEscape::escape(em.callerPhone) << "\","
           << "\"pickupLocation\":\"" << JsonEscape::escape(em.pickupLocation) << "\","
           << "\"location\":\"" << JsonEscape::escape(em.pickupLocation) << "\","
           << "\"latitude\":" << em.latitude << ","
           << "\"longitude\":" << em.longitude << ","
           << "\"severity\":\"" << JsonEscape::escape(em.severity) << "\","
           << "\"triageCategory\":\"" << JsonEscape::escape(em.severity) << "\","
           << "\"conditionText\":\"" << JsonEscape::escape(em.conditionText) << "\","
           << "\"condition\":\"" << JsonEscape::escape(em.conditionText) << "\","
           << "\"status\":\"" << JsonEscape::escape(em.status) << "\","
           << "\"assignedHospitalId\":\"" << JsonEscape::escape(em.assignedHospitalId) << "\","
           << "\"assignedAmbulanceId\":\"" << JsonEscape::escape(em.assignedAmbulanceId) << "\","
           << "\"createdAt\":\"" << JsonEscape::escape(em.createdAt) << "\","
           << "\"updatedAt\":\"" << JsonEscape::escape(em.updatedAt) << "\"";

        std::optional<AmbulanceRecord> amb;
        if (ambulanceRepo && !em.assignedAmbulanceId.empty()) {
            amb = ambulanceRepo->findById(em.assignedAmbulanceId);
            if (amb.has_value()) {
                ss << ",\"ambulance\":{"
                   << "\"id\":\"" << JsonEscape::escape(amb->ambulanceId) << "\","
                   << "\"ambulanceId\":\"" << JsonEscape::escape(amb->ambulanceId) << "\","
                   << "\"unitCode\":\"" << JsonEscape::escape(amb->unitCode) << "\","
                   << "\"type\":\"" << JsonEscape::escape(amb->type) << "\","
                   << "\"driverName\":\"" << JsonEscape::escape(amb->driverName) << "\","
                   << "\"driverPhone\":\"" << JsonEscape::escape(amb->driverPhone) << "\","
                   << "\"currentLat\":" << amb->currentLat << ","
                   << "\"currentLng\":" << amb->currentLng << ","
                   << "\"latitude\":" << amb->currentLat << ","
                   << "\"longitude\":" << amb->currentLng << ","
                   << "\"status\":\"" << JsonEscape::escape(amb->status) << "\""
                   << "}";
            }
        }

        std::optional<HospitalRecord> hosp;
        if (hospitalRepo && !em.assignedHospitalId.empty()) {
            hosp = hospitalRepo->findById(em.assignedHospitalId);
            if (hosp.has_value()) {
                ss << ",\"hospital\":{"
                   << "\"id\":\"" << JsonEscape::escape(hosp->hospitalId) << "\","
                   << "\"hospitalId\":\"" << JsonEscape::escape(hosp->hospitalId) << "\","
                   << "\"name\":\"" << JsonEscape::escape(hosp->name) << "\","
                   << "\"address\":\"" << JsonEscape::escape(hosp->address) << "\","
                   << "\"latitude\":" << hosp->latitude << ","
                   << "\"longitude\":" << hosp->longitude << ","
                   << "\"phone\":\"" << JsonEscape::escape(hosp->phone) << "\","
                   << "\"availableIcu\":" << hosp->availableIcu << ","
                   << "\"totalBeds\":" << hosp->totalBeds
                   << "}";
            }
        }

        // Live Dijkstra route recalculation based on current ambulance coordinates
        if (graph && pathFinder && amb.has_value() && amb->currentLat != 0.0 && amb->currentLng != 0.0 &&
            em.status != "COMPLETED" && em.status != "CANCELLED") {

            std::string ambNode = graph->findNearestNodeId(amb->currentLat, amb->currentLng);
            std::string targetNode;
            std::string targetPhase;

            if (em.status == "TRANSPORTING" || em.status == "ARRIVED") {
                targetNode = em.assignedHospitalId;
                targetPhase = "TO_HOSPITAL";
            } else {
                targetNode = graph->findNearestNodeId(em.latitude, em.longitude);
                targetPhase = "TO_PATIENT";
            }

            if (!ambNode.empty() && !targetNode.empty()) {
                double distKm = 0.0;
                double etaMins = 0.0;
                std::vector<std::string> pathNodes;

                if (ambNode == targetNode) {
                    distKm = 0.2;
                    etaMins = 0.5;
                    pathNodes = {ambNode};
                } else {
                    auto routeRes = pathFinder->findShortestRoute(ambNode, targetNode);
                    if (routeRes.isSuccess()) {
                        auto route = routeRes.getData();
                        if (transportEstimator) {
                            transportEstimator->estimateAll(route);
                        }
                        distKm = route.getTotalDistanceKm();
                        auto times = route.getTravelTimes();
                        auto it = times.find("Ambulance");
                        if (it != times.end() && it->second > 0) {
                            etaMins = it->second;
                        } else {
                            etaMins = (distKm / 45.0) * 60.0;
                        }
                        pathNodes = route.getPathNodeIds();
                    }
                }

                ss << ",\"ambulanceEtaMinutes\":" << etaMins
                   << ",\"ambulanceDistanceKm\":" << distKm
                   << ",\"liveRoute\":{"
                   << "\"active\":true,"
                   << "\"phase\":\"" << targetPhase << "\","
                   << "\"distanceKm\":" << distKm << ","
                   << "\"etaMinutes\":" << etaMins << ","
                   << "\"pathNodeIds\":[";
                for (size_t p = 0; p < pathNodes.size(); ++p) {
                    ss << "\"" << JsonEscape::escape(pathNodes[p]) << "\"";
                    if (p + 1 < pathNodes.size()) ss << ",";
                }
                ss << "],\"coordinates\":[";
                for (size_t p = 0; p < pathNodes.size(); ++p) {
                    double nLat = 0.0, nLng = 0.0;
                    std::string nName = pathNodes[p];
                    if (locationRepo) {
                        auto loc = locationRepo->findById(pathNodes[p]);
                        if (loc) {
                            nLat = loc->getLatitude();
                            nLng = loc->getLongitude();
                            nName = loc->getName();
                        }
                    }
                    ss << "{\"nodeId\":\"" << JsonEscape::escape(pathNodes[p]) << "\","
                       << "\"latitude\":" << nLat << ","
                       << "\"longitude\":" << nLng << ","
                       << "\"name\":\"" << JsonEscape::escape(nName) << "\"}";
                    if (p + 1 < pathNodes.size()) ss << ",";
                }
                ss << "]}";
            }
        }

        ss << "}";
        return ss.str();
    };

    // Handler for single emergency details & tracking
    auto handleEmergencyGet = [this, formatEmergencyDetailsJson](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::string emId = req.matches[1];

        std::optional<EmergencyRecord> emOpt;
        if (emergencyRepo) {
            emOpt = emergencyRepo->findById(emId);
        }

        if (!emOpt.has_value()) {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Emergency ID not found: " + emId), "application/json");
            return;
        }

        const auto& em = *emOpt;

        // Authentication check: Bearer token or anonymous tracking token
        std::string token = extractBearerToken(req);
        if (!token.empty() && authService) {
            auto userOpt = authService->validateToken(token);
            if (!userOpt.has_value()) {
                res.status = 401;
                res.set_content(buildJsonError("UNAUTHORIZED", "Invalid or expired token"), "application/json");
                return;
            }

            const auto& user = *userOpt;
            if (user.role == "ADMIN") {
                // Admin has full visibility
            } else if (user.role == "PATIENT") {
                if (em.userId == 0 || em.userId != user.userId) {
                    res.status = 403;
                    res.set_content(buildJsonError("FORBIDDEN", "Access denied: you do not own emergency " + emId), "application/json");
                    return;
                }
            } else if (user.role == "DRIVER") {
                if (em.assignedAmbulanceId != user.linkedAmbulanceId) {
                    res.status = 403;
                    res.set_content(buildJsonError("FORBIDDEN", "Access denied: emergency not assigned to your ambulance"), "application/json");
                    return;
                }
            } else if (user.role == "HOSPITAL") {
                if (em.assignedHospitalId != user.linkedHospitalId) {
                    res.status = 403;
                    res.set_content(buildJsonError("FORBIDDEN", "Access denied: emergency not routed to your hospital"), "application/json");
                    return;
                }
            } else {
                res.status = 403;
                res.set_content(buildJsonError("FORBIDDEN", "Access denied: insufficient permissions"), "application/json");
                return;
            }
        } else {
            // Anonymous tracking token validation
            std::string trackingToken = req.get_header_value("X-Tracking-Token");
            if (trackingToken.empty() && req.has_param("token")) {
                trackingToken = req.get_param_value("token");
            }

            if (trackingToken.empty()) {
                res.status = 401;
                res.set_content(buildJsonError("UNAUTHORIZED", "Authentication or tracking token required to view emergency details"), "application/json");
                return;
            }

            if (em.trackingTokenHash.empty()) {
                res.status = 403;
                res.set_content(buildJsonError("FORBIDDEN", "No tracking token associated with this emergency"), "application/json");
                return;
            }

            std::string providedHash = CryptoUtil::sha256Hex(trackingToken);
            if (!CryptoUtil::constantTimeCompare(providedHash, em.trackingTokenHash)) {
                res.status = 403;
                res.set_content(buildJsonError("FORBIDDEN", "Invalid tracking token"), "application/json");
                return;
            }
        }

        std::string detailsJson = formatEmergencyDetailsJson(em);
        std::ostringstream ss;
        ss << "{\"success\":true,\"data\":" << detailsJson
           << ",\"emergency\":" << detailsJson << "}";
        res.set_content(ss.str(), "application/json");
    };

    server->Get(R"(/api/emergencies/([^/]+))", handleEmergencyGet);
    server->Get(R"(/api/emergencies/([^/]+)/tracking)", handleEmergencyGet);

    // 2. GET /nearby?lat=...&lng=...&radius=...&type=...
    server->Get("/nearby", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);

        if (!req.has_param("lat") || !req.has_param("lng")) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Missing required parameters: lat and lng"), "application/json");
            return;
        }

        try {
            double lat = std::stod(req.get_param_value("lat"));
            double lng = std::stod(req.get_param_value("lng"));
            double radius = req.has_param("radius") ? std::stod(req.get_param_value("radius")) : 5.0;

            LocationType filterType = LocationType::UNKNOWN;
            if (req.has_param("type")) {
                filterType = stringToLocationType(req.get_param_value("type"));
            }

            auto nearbyResult = nearbyFinder->findNearby(lat, lng, radius, filterType);

            if (!nearbyResult.isSuccess()) {
                if (nearbyResult.getErrorCode() == "NO_RESULTS") {
                    res.status = 200;
                    res.set_content("{\"success\":true,\"status\":\"NO_RESULTS\",\"message\":\"" + nearbyResult.getMessage() + "\",\"data\":[]}", "application/json");
                } else {
                    res.status = 400;
                    res.set_content(buildJsonError(nearbyResult.getErrorCode(), nearbyResult.getMessage()), "application/json");
                }
                return;
            }

            std::ostringstream ss;
            ss << "{\"success\":true,\"status\":\"OK\",\"message\":\"" << nearbyResult.getMessage() << "\",\"data\":"
               << NearbyServicesFinder::resultsToJson(nearbyResult.getData()) << "}";
            res.set_content(ss.str(), "application/json");

        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", std::string("Invalid parameter format: ") + e.what()), "application/json");
        }
    });

    // 3. GET /route?from=...&to=...&mode=...
    server->Get("/route", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);

        if (!req.has_param("from") || !req.has_param("to")) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Missing required query parameters: 'from' and 'to'"), "application/json");
            return;
        }

        std::string from = req.get_param_value("from");
        std::string to = req.get_param_value("to");

        if (req.has_param("fromLat") && req.has_param("fromLng")) {
            try {
                double fLat = std::stod(req.get_param_value("fromLat"));
                double fLng = std::stod(req.get_param_value("fromLng"));
                from = graph->findNearestNodeId(fLat, fLng);
            } catch (...) {}
        }
        if (req.has_param("toLat") && req.has_param("toLng")) {
            try {
                double tLat = std::stod(req.get_param_value("toLat"));
                double tLng = std::stod(req.get_param_value("toLng"));
                to = graph->findNearestNodeId(tLat, tLng);
            } catch (...) {}
        }

        auto routeResult = pathFinder->findShortestRoute(from, to);

        if (!routeResult.isSuccess()) {
            res.status = (routeResult.getErrorCode() == "NOT_FOUND") ? 404 : 400;
            res.set_content(buildJsonError(routeResult.getErrorCode(), routeResult.getMessage()), "application/json");
            return;
        }

        RouteResponse route = routeResult.getData();
        if (transportEstimator) {
            transportEstimator->estimateAll(route);
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"message\":\"Shortest path computed successfully\",\"data\":"
           << route.toJson() << "}";
        res.set_content(ss.str(), "application/json");
    });

    // 4. GET /hospitals and /api/hospitals
    auto handleHospitals = [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        std::vector<HospitalRecord> hospitals;
        if (hospitalRepo) {
            hospitals = hospitalRepo->findAll();
        }
        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"count\":" << hospitals.size() << ",\"data\":[";
        for (size_t i = 0; i < hospitals.size(); ++i) {
            ss << hospitalToJson(hospitals[i]);
            if (i + 1 < hospitals.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    };
    server->Get("/hospitals", handleHospitals);
    server->Get("/api/hospitals", handleHospitals);

    // 5. GET /api/hospitals/search and /hospitals/search
    auto handleHospitalSearch = [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::string query = req.has_param("query") ? req.get_param_value("query") : "";
        std::string specialty = req.has_param("specialty") ? req.get_param_value("specialty") : "";

        std::vector<HospitalRecord> matched;
        if (hospitalRepo) {
            matched = hospitalRepo->search(query, specialty, false);
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"count\":" << matched.size() << ",\"data\":[";
        for (size_t i = 0; i < matched.size(); ++i) {
            ss << hospitalToJson(matched[i]);
            if (i + 1 < matched.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    };
    server->Get("/api/hospitals/search", handleHospitalSearch);
    server->Get("/hospitals/search", handleHospitalSearch);

    // 6. Emergency Dispatch: POST /api/emergency-requests (Public SOS with rate limiting & tracking tokens)
    server->Post("/api/emergency-requests", [this, extractStr, extractNum, isMalformedJson](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);

        if (req.body.size() > 65536) {
            res.status = 413;
            res.set_content(buildJsonError("PAYLOAD_TOO_LARGE", "Request body exceeds 64KB limit"), "application/json");
            return;
        }

        if (isMalformedJson(req.body)) {
            res.status = 400;
            res.set_content(buildJsonError("MALFORMED_JSON", "Request body is not valid JSON"), "application/json");
            return;
        }

        std::string body = req.body;
        std::string name = extractStr(body, "name");
        if (name.empty()) name = extractStr(body, "callerName");
        if (name.empty()) name = extractStr(body, "patientName");
        std::string phone = extractStr(body, "phone");
        if (phone.empty()) phone = extractStr(body, "callerPhone");
        if (phone.empty()) phone = extractStr(body, "patientPhone");
        std::string location = extractStr(body, "location");
        if (location.empty()) location = extractStr(body, "pickupLocation");
        std::string severity = extractStr(body, "severity");
        std::string conditionText = extractStr(body, "conditionText");
        if (conditionText.empty()) conditionText = extractStr(body, "condition");

        // Optional patient association via Bearer token
        int64_t patientUserId = 0;
        std::string token = extractBearerToken(req);
        if (!token.empty() && authService) {
            auto userOpt = authService->validateToken(token);
            if (userOpt.has_value()) {
                patientUserId = userOpt->userId;
                if ((name.empty() || name == "Emergency Caller") && !userOpt->displayName.empty()) {
                    name = userOpt->displayName;
                }
                if (phone.empty() && !userOpt->phone.empty()) {
                    phone = userOpt->phone;
                }
            }
        }

        // Enforce SOS rate limiting (5 per 10min for anonymous, 20 per 10min for authenticated users)
        std::string clientIp = req.remote_addr;
        if (!g_sosRateLimiter.checkAndRecord(clientIp, patientUserId)) {
            res.status = 429;
            res.set_content(buildJsonError("RATE_LIMIT_EXCEEDED", "Too many SOS requests. Please try again later."), "application/json");
            return;
        }

        // Generate tracking token for anonymous caller
        std::string plainTrackingToken = "";
        std::string trackingTokenHash = "";
        if (patientUserId == 0) {
            plainTrackingToken = CryptoUtil::generateToken(16); // 32 hex chars
            trackingTokenHash = CryptoUtil::sha256Hex(plainTrackingToken);
        }

        if (name.empty()) name = "Emergency Caller";
        if (location.empty()) location = "Clock Tower, Dehradun";

        double lat = extractNum(body, "latitude", 30.3244);
        double lng = extractNum(body, "longitude", 78.0418);

        if (std::isnan(lat) || std::isnan(lng) || std::isinf(lat) || std::isinf(lng) ||
            lat < -90.0 || lat > 90.0 || lng < -180.0 || lng > 180.0 || (lat == 0.0 && lng == 0.0)) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Invalid GPS coordinates for emergency: latitude must be [-90, 90], longitude [-180, 180], non-zero"), "application/json");
            return;
        }

        if (dispatcher) {
            auto dRes = dispatcher->dispatchEmergency(name, phone, location, lat, lng, conditionText, severity, patientUserId, trackingTokenHash);
            if (!dRes.success) {
                res.status = (dRes.errorCode == "NO_AMBULANCE_AVAILABLE" || dRes.errorCode == "NO_SUITABLE_HOSPITAL") ? 503 : 400;
                res.set_content(buildJsonError(dRes.errorCode, dRes.message), "application/json");
                return;
            }

            std::ostringstream ss;
            ss << "{\"success\":true,\"status\":\"OK\",\"message\":\"" << JsonEscape::escape(dRes.message) << "\","
               << "\"severity\":\"" << JsonEscape::escape(dRes.emergency.severity) << "\",";
            if (!plainTrackingToken.empty()) {
                ss << "\"trackingToken\":\"" << JsonEscape::escape(plainTrackingToken) << "\",";
            }
            ss << "\"data\":{"
               << "\"emergencyId\":\"" << JsonEscape::escape(dRes.emergency.emergencyId) << "\","
               << "\"status\":\"" << JsonEscape::escape(dRes.emergency.status) << "\","
               << "\"severity\":\"" << JsonEscape::escape(dRes.emergency.severity) << "\",";
            if (!plainTrackingToken.empty()) {
                ss << "\"trackingToken\":\"" << JsonEscape::escape(plainTrackingToken) << "\",";
            }
            ss << "\"assignedAmbulance\":\"" << JsonEscape::escape(dRes.assignedAmbulance.ambulanceId) << "\","
               << "\"assignedAmbulanceId\":\"" << JsonEscape::escape(dRes.assignedAmbulance.ambulanceId) << "\","
               << "\"assignedHospital\":\"" << JsonEscape::escape(dRes.assignedHospital.hospitalId) << "\","
               << "\"assignedHospitalId\":\"" << JsonEscape::escape(dRes.assignedHospital.hospitalId) << "\","
               << "\"etaMinutes\":" << dRes.ambulanceEtaMinutes << ","
               << "\"triage\":{"
               << "\"severity\":\"" << JsonEscape::escape(emergencySeverityToString(dRes.triage.severity)) << "\","
               << "\"recommendedSpecialty\":\"" << JsonEscape::escape(dRes.triage.recommendedSpecialty) << "\","
               << "\"requiredAmbulanceType\":\"" << JsonEscape::escape(dRes.triage.requiredAmbulanceType) << "\","
               << "\"clinicalProtocol\":\"" << JsonEscape::escape(dRes.triage.clinicalProtocol) << "\","
               << "\"urgencyScore\":" << dRes.triage.urgencyScore
               << "},"
               << "\"ambulance\":{"
               << "\"id\":\"" << JsonEscape::escape(dRes.assignedAmbulance.ambulanceId) << "\","
               << "\"unitCode\":\"" << JsonEscape::escape(dRes.assignedAmbulance.unitCode) << "\","
               << "\"type\":\"" << JsonEscape::escape(dRes.assignedAmbulance.type) << "\","
               << "\"driverName\":\"" << JsonEscape::escape(dRes.assignedAmbulance.driverName) << "\","
               << "\"driverPhone\":\"" << JsonEscape::escape(dRes.assignedAmbulance.driverPhone) << "\","
               << "\"distanceKm\":" << dRes.ambulanceDistanceKm << ","
               << "\"etaMinutes\":" << dRes.ambulanceEtaMinutes
               << "},"
               << "\"hospital\":{"
               << "\"id\":\"" << JsonEscape::escape(dRes.assignedHospital.hospitalId) << "\","
               << "\"name\":\"" << JsonEscape::escape(dRes.assignedHospital.name) << "\","
               << "\"distanceKm\":" << dRes.hospitalDistanceKm << ","
               << "\"etaMinutes\":" << dRes.hospitalEtaMinutes
               << "}"
               << "}}";
            res.set_content(ss.str(), "application/json");
            return;
        }

        EmergencyRecord rec;
        rec.callerName = name;
        rec.callerPhone = phone;
        rec.pickupLocation = location;
        rec.latitude = lat;
        rec.longitude = lng;
        rec.severity = severity.empty() ? "HIGH" : severity;
        rec.conditionText = conditionText;
        rec.status = "DISPATCHED";
        rec.userId = patientUserId;
        rec.trackingTokenHash = trackingTokenHash;

        // Fallback round-robin if dispatcher is not initialized
        std::string assignedAmb = "AMB-01";
        std::string assignedHospital = "LOC-004"; // Max Super Speciality default

        if (ambulanceRepo) {
            auto ambulances = ambulanceRepo->findAll();
            static std::atomic<size_t> s_rrIndex{0};
            if (!ambulances.empty()) {
                size_t idx = s_rrIndex.fetch_add(1) % ambulances.size();
                // Prefer READY ambulances if available
                bool foundReady = false;
                for (size_t i = 0; i < ambulances.size(); ++i) {
                    size_t checkIdx = (idx + i) % ambulances.size();
                    if (ambulances[checkIdx].status == "READY") {
                        assignedAmb = ambulances[checkIdx].ambulanceId;
                        foundReady = true;
                        break;
                    }
                }
                if (!foundReady) {
                    assignedAmb = ambulances[idx].ambulanceId;
                }
            }
        }

        rec.assignedHospitalId = assignedHospital;
        rec.assignedAmbulanceId = assignedAmb;

        EmergencyRecord created;
        if (emergencyRepo) {
            created = emergencyRepo->create(rec);
            emergencyRepo->updateAssignment(created.emergencyId, assignedHospital, assignedAmb);
        } else {
            created = rec;
            created.emergencyId = "SOS-1001";
        }

        if (ambulanceRepo) {
            ambulanceRepo->updateStatus(assignedAmb, "DISPATCHED", created.emergencyId);
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"message\":\"Emergency request created successfully\","
           << "\"data\":{"
           << "\"emergencyId\":\"" << JsonEscape::escape(created.emergencyId) << "\","
           << "\"status\":\"DISPATCHED\","
           << "\"assignedAmbulance\":\"" << JsonEscape::escape(assignedAmb) << "\","
           << "\"assignedHospital\":\"" << JsonEscape::escape(assignedHospital) << "\","
           << "\"etaMinutes\":7.5"
           << "}}";
        res.set_content(ss.str(), "application/json");
    });

    // 7. GET /api/emergency-requests and /api/emergencies (ADMIN only)
    auto handleEmergencyRequests = [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        std::vector<EmergencyRecord> list;
        if (emergencyRepo) {
            list = emergencyRepo->findAll();
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"count\":" << list.size() << ",\"emergencies\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            ss << emergencyToJson(list[i]);
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "],\"data\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            ss << emergencyToJson(list[i]);
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    };
    server->Get("/api/emergency-requests", handleEmergencyRequests);
    server->Get("/api/emergencies", handleEmergencyRequests);

    // 8. PATCH /api/emergencies/:id/status (ADMIN only dispatcher override)
    server->Patch(R"(/api/emergencies/([^/]+)/status)", [this, extractStr](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        std::string emId = req.matches[1];

        std::string newStatus = "ARRIVED";
        std::string parsedStatus = extractStr(req.body, "status");
        if (!parsedStatus.empty()) {
            newStatus = parsedStatus;
        } else if (req.body.find("ASSIGNED") != std::string::npos) {
            newStatus = "ASSIGNED";
        } else if (req.body.find("DISPATCHED") != std::string::npos) {
            newStatus = "DISPATCHED";
        } else if (req.body.find("ON_SCENE") != std::string::npos) {
            newStatus = "ON_SCENE";
        } else if (req.body.find("TRANSPORTING") != std::string::npos) {
            newStatus = "TRANSPORTING";
        } else if (req.body.find("ARRIVED") != std::string::npos) {
            newStatus = "ARRIVED";
        } else if (req.body.find("COMPLETED") != std::string::npos) {
            newStatus = "COMPLETED";
        } else if (req.body.find("CANCELLED") != std::string::npos) {
            newStatus = "CANCELLED";
        }

        if (dispatcher) {
            auto tRes = dispatcher->transitionStatus(emId, newStatus, "Status updated via API");
            if (!tRes.success) {
                res.status = (tRes.errorCode == "NOT_FOUND") ? 404 : 400;
                res.set_content(buildJsonError(tRes.errorCode, tRes.message), "application/json");
                return;
            }

            std::ostringstream ss;
            ss << "{\"success\":true,\"message\":\"" << JsonEscape::escape(tRes.message) << "\","
               << "\"data\":{"
               << "\"emergencyId\":\"" << JsonEscape::escape(tRes.emergencyId) << "\","
               << "\"oldStatus\":\"" << JsonEscape::escape(tRes.oldStatus) << "\","
               << "\"newStatus\":\"" << JsonEscape::escape(tRes.newStatus) << "\","
               << "\"ambulanceId\":\"" << JsonEscape::escape(tRes.ambulanceId) << "\","
               << "\"ambulanceStatus\":\"" << JsonEscape::escape(tRes.ambulanceStatus) << "\""
               << "}}";
            res.set_content(ss.str(), "application/json");
            return;
        }

        bool updated = false;
        if (emergencyRepo) {
            updated = emergencyRepo->updateStatus(emId, newStatus, "Status updated via API");
        }

        if (updated) {
            std::ostringstream ss;
            ss << "{\"success\":true,\"message\":\"Status updated to " << JsonEscape::escape(newStatus) << "\","
               << "\"data\":{\"emergencyId\":\"" << JsonEscape::escape(emId) << "\",\"newStatus\":\"" << JsonEscape::escape(newStatus) << "\"}}";
            res.set_content(ss.str(), "application/json");
        } else {
            res.status = 404;
            res.set_content(buildJsonError("NOT_FOUND", "Emergency ID not found: " + emId), "application/json");
        }
    });

    // 9. GET /ambulance-location and /api/ambulances (ADMIN only)
    auto handleAmbulances = [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        std::vector<AmbulanceRecord> list;
        if (ambulanceRepo) {
            list = ambulanceRepo->findAll();
        }
        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"count\":" << list.size() << ",\"data\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            const auto& a = list[i];
            ss << "{"
               << "\"id\":\"" << JsonEscape::escape(a.ambulanceId) << "\","
               << "\"unitCode\":\"" << JsonEscape::escape(a.unitCode) << "\","
               << "\"type\":\"" << JsonEscape::escape(a.type) << "\","
               << "\"driver\":\"" << JsonEscape::escape(a.driverName) << "\","
               << "\"driverName\":\"" << JsonEscape::escape(a.driverName) << "\","
               << "\"phone\":\"" << JsonEscape::escape(a.driverPhone) << "\","
               << "\"driverPhone\":\"" << JsonEscape::escape(a.driverPhone) << "\","
               << "\"latitude\":" << a.currentLat << ","
               << "\"longitude\":" << a.currentLng << ","
               << "\"status\":\"" << JsonEscape::escape(a.status) << "\","
               << "\"assignedEmergency\":\"" << JsonEscape::escape(a.assignedEmergencyId) << "\""
               << "}";
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    };
    server->Get("/ambulance-location", handleAmbulances);
    server->Get("/api/ambulances", handleAmbulances);

    // POST /ambulance-location and /api/ambulance-location (DRIVER own ambulance or ADMIN)
    auto handleAmbulanceLocationPost = [this, extractStr, extractNum](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto userOpt = authenticate(req, res, {"DRIVER", "ADMIN"});
        if (!userOpt.has_value()) return;

        std::string ambId = extractStr(req.body, "ambulanceId");
        if (ambId.empty()) ambId = extractStr(req.body, "id");
        if (ambId.empty()) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Missing ambulanceId in location update"), "application/json");
            return;
        }

        if (userOpt->role == "DRIVER" && userOpt->linkedAmbulanceId != ambId) {
            res.status = 403;
            res.set_content(buildJsonError("FORBIDDEN", "Access denied: cannot update location for another ambulance unit."), "application/json");
            return;
        }

        double lat = extractNum(req.body, "latitude", 30.3244);
        double lng = extractNum(req.body, "longitude", 78.0418);
        if (std::isnan(lat) || std::isnan(lng) || std::isinf(lat) || std::isinf(lng) ||
            lat < -90.0 || lat > 90.0 || lng < -180.0 || lng > 180.0 || (lat == 0.0 && lng == 0.0)) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Invalid GPS coordinates: latitude must be [-90, 90], longitude [-180, 180], non-zero"), "application/json");
            return;
        }

        if (ambulanceRepo) {
            ambulanceRepo->updatePosition(ambId, lat, lng);
            ambulanceRepo->appendLocation(ambId, lat, lng);
        }

        res.set_content("{\"success\":true,\"message\":\"Ambulance location updated for " + ambId + "\"}", "application/json");
    };
    server->Post("/ambulance-location", handleAmbulanceLocationPost);
    server->Post("/api/ambulance-location", handleAmbulanceLocationPost);

    // 10. Real-Time Patient Location Tracking: POST /api/patient-location (PATIENT or ADMIN)
    server->Post("/api/patient-location", [this, extractStr, extractNum](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto userOpt = authenticate(req, res, {"PATIENT", "ADMIN"});
        if (!userOpt.has_value()) return;

        std::string body = req.body;
        std::string emId = extractStr(body, "emergencyId");
        if (emId.empty()) emId = "PATIENT-LIVE-01";
        std::string name = extractStr(body, "patientName");
        if (name.empty()) {
            name = !userOpt->displayName.empty() ? userOpt->displayName : "Patient (Live GPS)";
        }
        double lat = extractNum(body, "latitude", 30.3244);
        double lng = extractNum(body, "longitude", 78.0418);

        if (std::isnan(lat) || std::isnan(lng) || std::isinf(lat) || std::isinf(lng) ||
            lat < -90.0 || lat > 90.0 || lng < -180.0 || lng > 180.0 || (lat == 0.0 && lng == 0.0)) {
            res.status = 400;
            res.set_content(buildJsonError("VALIDATION_ERROR", "Invalid GPS coordinates for emergency: latitude must be [-90, 90], longitude [-180, 180], non-zero"), "application/json");
            return;
        }
        double accuracy = extractNum(body, "accuracy", 5.0);

        std::string nearestNode = graph ? graph->findNearestNodeId(lat, lng) : "LOC-001";

        PatientLocationRecord prec;
        prec.emergencyId = emId;
        prec.patientName = name;
        prec.latitude = lat;
        prec.longitude = lng;
        prec.accuracyM = accuracy;
        prec.nearestNodeId = nearestNode;

        if (patientRepo) {
            patientRepo->upsert(prec);
        }

        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"message\":\"Real-time patient location updated\","
           << "\"data\":{"
           << "\"emergencyId\":\"" << JsonEscape::escape(emId) << "\","
           << "\"latitude\":" << lat << ","
           << "\"longitude\":" << lng << ","
           << "\"accuracyMeters\":" << accuracy << ","
           << "\"nearestGraphNodeId\":\"" << JsonEscape::escape(nearestNode) << "\""
           << "}}";
        res.set_content(ss.str(), "application/json");
    });

    // GET /api/patient-location (ADMIN only)
    server->Get("/api/patient-location", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;

        std::vector<PatientLocationRecord> list;
        if (patientRepo) {
            list = patientRepo->findAll();
        }
        std::ostringstream ss;
        ss << "{\"success\":true,\"status\":\"OK\",\"count\":" << list.size() << ",\"data\":[";
        for (size_t i = 0; i < list.size(); ++i) {
            const auto& p = list[i];
            ss << "{"
               << "\"emergencyId\":\"" << JsonEscape::escape(p.emergencyId) << "\","
               << "\"patientName\":\"" << JsonEscape::escape(p.patientName) << "\","
               << "\"latitude\":" << p.latitude << ","
               << "\"longitude\":" << p.longitude << ","
               << "\"accuracyMeters\":" << p.accuracyM << ","
               << "\"nearestGraphNodeId\":\"" << JsonEscape::escape(p.nearestNodeId) << "\""
               << "}";
            if (i + 1 < list.size()) ss << ",";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
    });

    // 11. GET /api/analytics (ADMIN only)
    server->Get("/api/analytics", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto adminOpt = authenticate(req, res, {"ADMIN"});
        if (!adminOpt.has_value()) return;
        size_t totalLocations = locationRepo ? locationRepo->count() : 0;
        size_t totalHospitals = hospitalRepo ? hospitalRepo->count() : (locationRepo ? locationRepo->findByType(LocationType::HOSPITAL).size() : 0);
        size_t totalEdges = graph ? graph->getEdgeCount() : 0;
        size_t activeAmbulances = 0;
        if (ambulanceRepo) {
            auto ambs = ambulanceRepo->findAll();
            for (const auto& a : ambs) {
                if (a.status != "MAINTENANCE") activeAmbulances++;
            }
        } else {
            activeAmbulances = 4;
        }
        size_t dispatches = emergencyRepo ? emergencyRepo->count() : 0;

        std::ostringstream ss;
        ss << "{"
           << "\"success\":true,"
           << "\"data\":{"
           << "\"totalLocations\":" << totalLocations << ","
           << "\"totalHospitals\":" << totalHospitals << ","
           << "\"totalRoadEdges\":" << totalEdges << ","
           << "\"activeAmbulances\":" << activeAmbulances << ","
           << "\"avgResponseTimeMinutes\":7.5,"
           << "\"totalDispatches\":" << dispatches << ","
           << "\"systemStatus\":\"ONLINE\""
           << "}}";
        res.set_content(ss.str(), "application/json");
    });

    // Static File Serving for Frontend (m4_frontend)
    server->set_mount_point("/", "./m4_frontend");
    server->set_mount_point("/", "../m4_frontend");
}

void RestApiServer::start(bool blocking) {
    if (running.load()) return;
    running.store(true);

    std::cout << "\n========================================================" << std::endl;
    std::cout << " [JEEVAN MARG] REST API Server Starting..." << std::endl;
    std::cout << " Listening on: http://localhost:" << port << std::endl;
    std::cout << " Endpoints: /nearby , /route , /hospitals , /api/emergency-requests" << std::endl;
    std::cout << " Dashboard: http://localhost:" << port << "/index.html" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    if (blocking) {
        server->listen(host.c_str(), port);
        running.store(false);
    } else {
        serverThread = std::thread([this]() {
            bool ok = server->listen(host.c_str(), port);
            if (!ok) {
                std::cerr << "[ERROR] server->listen returned false for " << host << ":" << port << std::endl;
            }
            running.store(false);
        });
        server->wait_until_ready();
    }
}

void RestApiServer::stop() {
    if (running.load()) {
        server->stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
        running.store(false);
        std::cout << "[JEEVAN MARG] REST API Server stopped." << std::endl;
    }
}
