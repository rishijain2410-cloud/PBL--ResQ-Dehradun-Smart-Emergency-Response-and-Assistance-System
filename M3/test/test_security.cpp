#include "../persistence/Database.h"
#include "../persistence/DatabaseBootstrap.h"
#include "../persistence/SqliteLocationRepository.h"
#include "../persistence/SqliteRoadRepository.h"
#include "../persistence/SqliteHospitalRepository.h"
#include "../persistence/SqliteAmbulanceRepository.h"
#include "../persistence/SqliteEmergencyRepository.h"
#include "../persistence/SqlitePatientLocationRepository.h"
#include "../persistence/SqliteUserRepository.h"
#include "../persistence/SqliteSessionRepository.h"
#include "../service/RoadGraphBuilder.h"
#include "../service/ShortestPathFinder.h"
#include "../service/NearbyServicesFinder.h"
#include "../service/TransportEstimator.h"
#include "../service/EmergencyDispatcher.h"
#include "../service/AuthService.h"
#include "../service/RestApiServer.h"
#include "../include/httplib.h"
#include "../util/CryptoUtil.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

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

static std::string locateSchema() {
    std::vector<std::string> searchPaths = {
        "database/schema.sql",
        "m3_navigation/database/schema.sql",
        "../database/schema.sql",
        "../../database/schema.sql"
    };
    for (const auto& p : searchPaths) {
        if (fs::exists(p)) return p;
    }
    return "database/schema.sql";
}

int main() {
    std::cout << "\n========================================================================\n";
    std::cout << " [JEEVAN MARG] SECURITY & TRIAGE INTEGRATION SUITE (Parts A & B)\n";
    std::cout << "========================================================================\n\n";

    // 1. Setup isolated test database
    std::string testDbPath = "data/test_security_eval.db";
    if (fs::exists(testDbPath)) {
        fs::remove(testDbPath);
    }
    fs::create_directories("data");

    std::string seedDir = "seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "m3_navigation/seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "../seed";

    std::string schemaFile = locateSchema();

    Database db;
    db.open(testDbPath);
    db.execFile(schemaFile);
    DatabaseBootstrap::initializeAndSeed(db, testDbPath, seedDir);
    DatabaseBootstrap::seedDemoUsers(db);

    auto locRepo = std::make_shared<SqliteLocationRepository>(db);
    auto roadRepo = std::make_shared<SqliteRoadRepository>(db);
    auto hospRepo = std::make_shared<SqliteHospitalRepository>(db);
    auto ambRepo = std::make_shared<SqliteAmbulanceRepository>(db);
    auto emerRepo = std::make_shared<SqliteEmergencyRepository>(db);
    auto patRepo = std::make_shared<SqlitePatientLocationRepository>(db);
    auto userRepo = std::make_shared<SqliteUserRepository>(db);
    auto sessionRepo = std::make_shared<SqliteSessionRepository>(db);

    auto edges = roadRepo->findAll();
    auto graph = RoadGraphBuilder::buildGraph(*locRepo, edges, true);
    auto pathFinder = std::make_shared<ShortestPathFinder>(graph);
    auto nearbyFinder = std::make_shared<NearbyServicesFinder>(locRepo);
    auto estimator = std::make_shared<TransportEstimator>();
    auto dispatcher = std::make_shared<EmergencyDispatcher>(db, emerRepo, ambRepo, hospRepo, graph, pathFinder, estimator);
    auto authService = std::make_shared<AuthService>(userRepo, sessionRepo, ambRepo, hospRepo);

    // 2. Start RestApiServer on test port 8095
    int testPort = 8095;
    auto dbPtr = std::shared_ptr<Database>(&db, [](Database*){});
    auto server = std::make_unique<RestApiServer>(
        testPort, pathFinder, nearbyFinder, estimator,
        locRepo, graph, hospRepo, ambRepo, emerRepo, patRepo,
        dispatcher, authService, userRepo, "0.0.0.0", dbPtr);

    server->start(false); // background thread
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    httplib::Client cli("127.0.0.1", testPort);
    cli.set_connection_timeout(5);
    cli.set_read_timeout(5);

    // 3. Obtain authentication tokens for each role
    auto adminLogin = authService->login("admin", "Admin@JeevanMarg2026!");
    TEST_ASSERT(adminLogin.success, "Admin login succeeded");
    std::string adminToken = adminLogin.token;

    auto hospMaxLogin = authService->login("hosp_max", "Hospital@123");
    TEST_ASSERT(hospMaxLogin.success, "Hospital Max login succeeded");
    std::string hospMaxToken = hospMaxLogin.token;

    auto hospDoonLogin = authService->login("hosp_doon", "Hospital@123");
    TEST_ASSERT(hospDoonLogin.success, "Hospital Doon login succeeded");
    std::string hospDoonToken = hospDoonLogin.token;

    auto driver1Login = authService->login("driver_amb01", "Driver@123");
    TEST_ASSERT(driver1Login.success, "Driver 1 (AMB-01) login succeeded");
    std::string driver1Token = driver1Login.token;

    auto driver2Login = authService->login("driver_amb02", "Driver@123");
    TEST_ASSERT(driver2Login.success, "Driver 2 (AMB-02) login succeeded");
    std::string driver2Token = driver2Login.token;

    auto patReg = authService->registerPatient("Demo Patient", "+91 99999 11111", "patient_sec", "PatientPass123!");
    TEST_ASSERT(patReg.success, "Patient registered");
    auto patLogin = authService->login("patient_sec", "PatientPass123!");
    TEST_ASSERT(patLogin.success, "Patient login succeeded");
    std::string patientToken = patLogin.token;

    httplib::Headers hNone = {};
    httplib::Headers hAdmin = {{"Authorization", "Bearer " + adminToken}};
    httplib::Headers hHospMax = {{"Authorization", "Bearer " + hospMaxToken}};
    httplib::Headers hHospDoon = {{"Authorization", "Bearer " + hospDoonToken}};
    httplib::Headers hDriver1 = {{"Authorization", "Bearer " + driver1Token}};
    httplib::Headers hDriver2 = {{"Authorization", "Bearer " + driver2Token}};
    httplib::Headers hPatient = {{"Authorization", "Bearer " + patientToken}};

    // =========================================================================
    // PART A.1: PUBLIC ENDPOINTS (No token required -> 200 OK)
    // =========================================================================
    std::cout << "[Security Matrix 1/5] Testing Public Endpoints...\n";
    {
        auto res = cli.Get("/api/health");
        if (!res) {
            std::cout << "[DEBUG] GET /api/health failed with error: " << httplib::to_string(res.error()) << "\n";
        } else {
            std::cout << "[DEBUG] GET /api/health returned status: " << res->status << "\n";
        }
        TEST_ASSERT(res && res->status == 200, "GET /api/health is public (200)");

        res = cli.Get("/api/hospitals");
        TEST_ASSERT(res && res->status == 200, "GET /api/hospitals is public (200)");

        res = cli.Get("/api/hospitals/search?query=Max");
        TEST_ASSERT(res && res->status == 200, "GET /api/hospitals/search is public (200)");

        res = cli.Get("/nearby?lat=30.3244&lng=78.0418&radius=5&type=HOSPITAL");
        TEST_ASSERT(res && res->status == 200, "GET /nearby is public (200)");

        res = cli.Get("/route?from=LOC-001&to=LOC-004");
        TEST_ASSERT(res && res->status == 200, "GET /route is public (200)");
    }

    // =========================================================================
    // PART A.2: ADMIN-ONLY ENDPOINTS (401 without token, 403 non-admin, 200 admin)
    // =========================================================================
    std::cout << "[Security Matrix 2/5] Testing ADMIN-only Endpoints...\n";
    {
        // GET /api/emergencies
        auto rNo = cli.Get("/api/emergencies", hNone);
        TEST_ASSERT(rNo && rNo->status == 401, "GET /api/emergencies no token -> 401");
        auto rPat = cli.Get("/api/emergencies", hPatient);
        TEST_ASSERT(rPat && rPat->status == 403, "GET /api/emergencies patient token -> 403");
        auto rAdm = cli.Get("/api/emergencies", hAdmin);
        TEST_ASSERT(rAdm && rAdm->status == 200, "GET /api/emergencies admin token -> 200");

        // GET /api/emergency-requests
        rNo = cli.Get("/api/emergency-requests", hNone);
        TEST_ASSERT(rNo && rNo->status == 401, "GET /api/emergency-requests no token -> 401");
        rPat = cli.Get("/api/emergency-requests", hPatient);
        TEST_ASSERT(rPat && rPat->status == 403, "GET /api/emergency-requests patient token -> 403");
        rAdm = cli.Get("/api/emergency-requests", hAdmin);
        TEST_ASSERT(rAdm && rAdm->status == 200, "GET /api/emergency-requests admin token -> 200");

        // GET /api/ambulances
        rNo = cli.Get("/api/ambulances", hNone);
        TEST_ASSERT(rNo && rNo->status == 401, "GET /api/ambulances no token -> 401");
        rPat = cli.Get("/api/ambulances", hPatient);
        TEST_ASSERT(rPat && rPat->status == 403, "GET /api/ambulances patient token -> 403");
        rAdm = cli.Get("/api/ambulances", hAdmin);
        TEST_ASSERT(rAdm && rAdm->status == 200, "GET /api/ambulances admin token -> 200");

        // GET /ambulance-location
        rNo = cli.Get("/ambulance-location", hNone);
        TEST_ASSERT(rNo && rNo->status == 401, "GET /ambulance-location no token -> 401");
        auto rDrv = cli.Get("/ambulance-location", hDriver1);
        TEST_ASSERT(rDrv && rDrv->status == 403, "GET /ambulance-location driver token -> 403");
        rAdm = cli.Get("/ambulance-location", hAdmin);
        TEST_ASSERT(rAdm && rAdm->status == 200, "GET /ambulance-location admin token -> 200");

        // GET /api/analytics
        rNo = cli.Get("/api/analytics", hNone);
        TEST_ASSERT(rNo && rNo->status == 401, "GET /api/analytics no token -> 401");
        auto rHosp = cli.Get("/api/analytics", hHospMax);
        TEST_ASSERT(rHosp && rHosp->status == 403, "GET /api/analytics hospital token -> 403");
        rAdm = cli.Get("/api/analytics", hAdmin);
        TEST_ASSERT(rAdm && rAdm->status == 200, "GET /api/analytics admin token -> 200");

        // GET /api/patient-location
        rNo = cli.Get("/api/patient-location", hNone);
        TEST_ASSERT(rNo && rNo->status == 401, "GET /api/patient-location no token -> 401");
        rPat = cli.Get("/api/patient-location", hPatient);
        TEST_ASSERT(rPat && rPat->status == 403, "GET /api/patient-location patient token -> 403");
        rAdm = cli.Get("/api/patient-location", hAdmin);
        TEST_ASSERT(rAdm && rAdm->status == 200, "GET /api/patient-location admin token -> 200");

        // POST /api/admin/reset-demo
        rNo = cli.Post("/api/admin/reset-demo", hNone, "{}", "application/json");
        TEST_ASSERT(rNo && rNo->status == 401, "POST /api/admin/reset-demo no token -> 401");
        rPat = cli.Post("/api/admin/reset-demo", hPatient, "{}", "application/json");
        TEST_ASSERT(rPat && rPat->status == 403, "POST /api/admin/reset-demo patient token -> 403");
        rAdm = cli.Post("/api/admin/reset-demo", hAdmin, "{}", "application/json");
        TEST_ASSERT(rAdm && rAdm->status == 200, "POST /api/admin/reset-demo admin token -> 200");
    }

    // =========================================================================
    // PART A.3: ROLE SCOPED & OWNERSHIP ENDPOINTS (Hospitals, Drivers, Patients)
    // =========================================================================
    std::cout << "[Security Matrix 3/5] Testing Role-Scoped Ownership Checks...\n";
    {
        // 1. PATCH /api/hospitals/:id/beds (LOC-004 is Max Hospital)
        std::string bedPayload = "{\"totalBeds\":220,\"icuTotal\":28,\"icuAvailable\":6,\"oxygenBeds\":45}";
        auto rNo = cli.Patch("/api/hospitals/LOC-004/beds", hNone, bedPayload, "application/json");
        TEST_ASSERT(rNo && rNo->status == 401, "PATCH hospital beds no token -> 401");

        auto rPat = cli.Patch("/api/hospitals/LOC-004/beds", hPatient, bedPayload, "application/json");
        TEST_ASSERT(rPat && rPat->status == 403, "PATCH hospital beds patient token -> 403");

        // Hospital B (Doon LOC-011) cannot edit Hospital A (Max LOC-004)
        auto rWrongHosp = cli.Patch("/api/hospitals/LOC-004/beds", hHospDoon, bedPayload, "application/json");
        TEST_ASSERT(rWrongHosp && rWrongHosp->status == 403, "Hospital B cannot edit Hospital A beds -> 403");

        // Hospital A (Max LOC-004) can edit own beds
        auto rOwnHosp = cli.Patch("/api/hospitals/LOC-004/beds", hHospMax, bedPayload, "application/json");
        TEST_ASSERT(rOwnHosp && rOwnHosp->status == 200, "Hospital A can edit own beds -> 200");

        // Admin can edit any hospital beds
        auto rAdminHosp = cli.Patch("/api/hospitals/LOC-004/beds", hAdmin, bedPayload, "application/json");
        TEST_ASSERT(rAdminHosp && rAdminHosp->status == 200, "Admin can edit hospital beds -> 200");

        // 2. POST /api/ambulance-location (AMB-01 vs AMB-02)
        std::string locPayload = "{\"ambulanceId\":\"AMB-01\",\"latitude\":30.3245,\"longitude\":78.0419}";
        rNo = cli.Post("/api/ambulance-location", hNone, locPayload, "application/json");
        TEST_ASSERT(rNo && rNo->status == 401, "POST ambulance-location no token -> 401");

        rPat = cli.Post("/api/ambulance-location", hPatient, locPayload, "application/json");
        TEST_ASSERT(rPat && rPat->status == 403, "POST ambulance-location patient token -> 403");

        // Driver 2 (AMB-02) cannot move AMB-01
        auto rWrongDrv = cli.Post("/api/ambulance-location", hDriver2, locPayload, "application/json");
        TEST_ASSERT(rWrongDrv && rWrongDrv->status == 403, "Driver 2 cannot update AMB-01 location -> 403");

        // Driver 1 (AMB-01) can update AMB-01
        auto rOwnDrv = cli.Post("/api/ambulance-location", hDriver1, locPayload, "application/json");
        TEST_ASSERT(rOwnDrv && rOwnDrv->status == 200, "Driver 1 can update AMB-01 location -> 200");

        // Admin can update any ambulance location
        auto rAdminDrv = cli.Post("/api/ambulance-location", hAdmin, locPayload, "application/json");
        TEST_ASSERT(rAdminDrv && rAdminDrv->status == 200, "Admin can update ambulance location -> 200");

        // 3. POST /api/patient-location
        std::string pLocPayload = "{\"emergencyId\":\"EMG-TEST\",\"patientName\":\"Ramesh\",\"latitude\":30.3244,\"longitude\":78.0418}";
        rNo = cli.Post("/api/patient-location", hNone, pLocPayload, "application/json");
        TEST_ASSERT(rNo && rNo->status == 401, "POST patient-location no token -> 401");

        rPat = cli.Post("/api/patient-location", hPatient, pLocPayload, "application/json");
        TEST_ASSERT(rPat && rPat->status == 200, "POST patient-location authenticated patient -> 200");
    }

    // =========================================================================
    // PART A.4: ANONYMOUS SOS DISPATCH & TRACKING TOKEN FLOW
    // =========================================================================
    std::cout << "[Security Matrix 4/5] Testing Anonymous SOS & Tracking Token Authentication...\n";
    {
        // 1. Dispatch anonymous SOS
        std::string sosBody = R"({
            "name": "Anonymous Bystander",
            "phone": "9876543210",
            "location": "Clock Tower, Dehradun",
            "latitude": 30.3244,
            "longitude": 78.0418,
            "conditionText": "Patient collapsed on street, unresponsive",
            "severity": "CRITICAL"
        })";

        auto sosRes = cli.Post("/api/emergency-requests", hNone, sosBody, "application/json");
        TEST_ASSERT(sosRes && sosRes->status == 200, "Anonymous SOS dispatch succeeds (200)");
        std::string resBody = (sosRes ? sosRes->body : "");
        TEST_ASSERT(resBody.find("trackingToken") != std::string::npos, "Response contains trackingToken");

        // Parse emergencyId and trackingToken from response
        size_t idPos = resBody.find("\"emergencyId\":\"");
        std::string emId = "";
        if (idPos != std::string::npos) {
            size_t idEnd = resBody.find("\"", idPos + 15);
            if (idEnd != std::string::npos) {
                emId = resBody.substr(idPos + 15, idEnd - (idPos + 15));
            }
        }
        TEST_ASSERT(!emId.empty(), "Dispatched emergency ID extracted");

        size_t tokPos = resBody.find("\"trackingToken\":\"");
        std::string trackingToken = "";
        if (tokPos != std::string::npos) {
            size_t tokEnd = resBody.find("\"", tokPos + 17);
            if (tokEnd != std::string::npos) {
                trackingToken = resBody.substr(tokPos + 17, tokEnd - (tokPos + 17));
            }
        }
        TEST_ASSERT(trackingToken.size() == 32, "Plain trackingToken is 32-hex random token");

        // 2. Anonymous read of emergency without token -> 401
        auto rNoTok = cli.Get("/api/emergencies/" + emId, hNone);
        TEST_ASSERT(rNoTok && rNoTok->status == 401, "GET emergency without token -> 401");

        // 3. Anonymous read with wrong token -> 403
        httplib::Headers hWrongTok = {{"X-Tracking-Token", "0123456789abcdef0123456789abcdef"}};
        auto rWrongTok = cli.Get("/api/emergencies/" + emId, hWrongTok);
        TEST_ASSERT(rWrongTok && rWrongTok->status == 403, "GET emergency with wrong tracking token -> 403");

        // 4. Anonymous read with correct header -> 200
        httplib::Headers hCorrectTok = {{"X-Tracking-Token", trackingToken}};
        auto rCorrectTok = cli.Get("/api/emergencies/" + emId, hCorrectTok);
        TEST_ASSERT(rCorrectTok && rCorrectTok->status == 200, "GET emergency with correct X-Tracking-Token header -> 200");

        // 5. Anonymous read with query parameter -> 200
        auto rQueryTok = cli.Get("/api/emergencies/" + emId + "?token=" + trackingToken);
        TEST_ASSERT(rQueryTok && rQueryTok->status == 200, "GET emergency with ?token= query parameter -> 200");

        // 6. Non-owner registered patient trying to access anonymous emergency -> 403
        auto rOtherPat = cli.Get("/api/emergencies/" + emId, hPatient);
        TEST_ASSERT(rOtherPat && rOtherPat->status == 403, "Non-owner patient accessing emergency -> 403");

        // 7. Admin can access emergency without tracking token -> 200
        auto rAdmAccess = cli.Get("/api/emergencies/" + emId, hAdmin);
        TEST_ASSERT(rAdmAccess && rAdmAccess->status == 200, "Admin can access emergency -> 200");

        // 8. Admin dispatcher override: PATCH /api/emergencies/:id/status
        auto rPatOverride = cli.Patch("/api/emergencies/" + emId + "/status", hPatient, "{\"status\":\"ARRIVED\"}", "application/json");
        TEST_ASSERT(rPatOverride && rPatOverride->status == 403, "Patient cannot use status override -> 403");

        auto rAdminOverride = cli.Patch("/api/emergencies/" + emId + "/status", hAdmin, "{\"status\":\"ON_SCENE\"}", "application/json");
        TEST_ASSERT(rAdminOverride && rAdminOverride->status == 200, "Admin status override succeeds -> 200");
    }

    // =========================================================================
    // PART B: TRIAGE SEVERITY & CLINICAL BED INTEGRITY
    // =========================================================================
    std::cout << "[Clinical Triage & Bed Integrity 5/5] Testing Severity Mapping & ICU Bed Reservations...\n";
    {
        // 1. "severe chest pain" -> CRITICAL, urgencyScore 1, ICU bed decremented by 1
        std::unordered_map<std::string, int> icuMap;
        for (const auto& hid : {"LOC-004", "LOC-005", "LOC-006", "LOC-007", "LOC-009", "LOC-011"}) {
            auto h = hospRepo->findById(hid);
            if (h) icuMap[hid] = h->availableIcu;
        }

        auto dChest = dispatcher->dispatchEmergency(
            "Cardiac Patient", "9876500001", "Clock Tower",
            30.3244, 78.0418,
            "severe chest pain radiating to left arm",
            ""); // Caller supplies no severity

        TEST_ASSERT(dChest.success, "Cardiac emergency dispatch succeeds");
        TEST_ASSERT(dChest.emergency.severity == "CRITICAL", "Triage engine source of truth: chest pain -> CRITICAL");
        TEST_ASSERT(dChest.triage.urgencyScore == 1, "Urgency score is 1 for CRITICAL");

        std::string assignedHospId = dChest.assignedHospital.hospitalId;
        auto hospAfter = hospRepo->findById(assignedHospId);
        TEST_ASSERT(hospAfter && hospAfter->availableIcu == icuMap[assignedHospId] - 1, "CRITICAL dispatch decrements available ICU by 1");

        // 2. Cancellation of CRITICAL emergency restores available ICU by 1
        auto cancelRes = dispatcher->transitionStatus(dChest.emergency.emergencyId, "CANCELLED", "Caller false alarm");
        TEST_ASSERT(cancelRes.success, "Transition to CANCELLED succeeds");
        auto hospRestored = hospRepo->findById(assignedHospId);
        TEST_ASSERT(hospRestored && hospRestored->availableIcu == icuMap[assignedHospId], "Emergency cancellation restores available ICU bed");

        // 3. "mild cold" -> LOW, urgencyScore 4, no ICU reserved
        auto dCold = dispatcher->dispatchEmergency(
            "Flu Patient", "9876500002", "Clock Tower",
            30.3244, 78.0418,
            "mild cold cough and fever",
            "");

        TEST_ASSERT(dCold.success, "Mild cold dispatch succeeds");
        TEST_ASSERT(dCold.emergency.severity == "LOW", "Triage engine source of truth: cold/cough -> LOW");
        TEST_ASSERT(dCold.triage.urgencyScore == 4, "Urgency score is 4 for LOW");
        auto hospCold = hospRepo->findById(dCold.assignedHospital.hospitalId);
        TEST_ASSERT(hospCold && hospCold->availableIcu == icuMap[dCold.assignedHospital.hospitalId], "LOW severity emergency does NOT reserve ICU bed");

        // 4. ICU Exhaustion Safety (Two simultaneous SOS cannot reserve more than available ICU)
        // Set Max Hospital available ICU to 1, others to 0 to simulate critical capacity limit
        hospRepo->updateBeds("LOC-004", 220, 28, 1, 45);
        hospRepo->updateBeds("LOC-005", 1500, 120, 0, 250);
        hospRepo->updateBeds("LOC-006", 180, 45, 0, 60);
        hospRepo->updateBeds("LOC-007", 500, 40, 0, 80);
        hospRepo->updateBeds("LOC-009", 120, 20, 0, 30);
        hospRepo->updateBeds("LOC-011", 350, 30, 0, 50);

        auto dCrit1 = dispatcher->dispatchEmergency(
            "Crit 1", "9000000001", "Clock Tower",
            30.3244, 78.0418, "unconscious patient not breathing", "CRITICAL");
        TEST_ASSERT(dCrit1.success, "First critical dispatch takes final available ICU bed");

        auto checkAfter1 = hospRepo->findById("LOC-004");
        TEST_ASSERT(checkAfter1->availableIcu == 0, "Available ICU bed is now 0");

        auto dCrit2 = dispatcher->dispatchEmergency(
            "Crit 2", "9000000002", "Clock Tower",
            30.3244, 78.0418, "cardiac arrest no pulse", "CRITICAL");
        TEST_ASSERT(!dCrit2.success, "Second critical dispatch rejected gracefully (no ICU available)");
        TEST_ASSERT(dCrit2.errorCode == "NO_SUITABLE_HOSPITAL" || dCrit2.errorCode == "NO_ICU_AVAILABLE", "Error code reflects capacity shortage");

        auto checkAfter2 = hospRepo->findById("LOC-004");
        TEST_ASSERT(checkAfter2->availableIcu == 0, "ICU bed count never drops below zero (no over-allocation)");

        // Restore demo data
        server->resetDemoData();
    }

    server->stop();

    std::cout << "\n========================================================================\n";
    std::cout << " RESULTS: " << g_testsPassed << " / " << g_testsRun << " Assertions Passed.\n";
    std::cout << "========================================================================\n\n";

    if (g_testsPassed == g_testsRun) {
        std::cout << "[PASS] All Security Lockdown & Triage Integrity Tests Passed!\n";
        return 0;
    } else {
        std::cerr << "[FAIL] Some tests failed!\n";
        return 1;
    }
}
