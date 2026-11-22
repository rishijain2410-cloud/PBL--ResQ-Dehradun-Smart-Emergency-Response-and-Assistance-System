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
#include "../util/CryptoUtil.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

static int g_testsRun = 0;
static int g_testsPassed = 0;
static int64_t g_sunitaUserId = 0;
static int64_t g_rajeshUserId = 0;

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

// =============================================================================
// Test Suite 1: Patient Portal Integration (Tests 1, 4, 5, 6)
// =============================================================================
void testPatientPortalWorkflow(
    std::shared_ptr<AuthService> authService,
    std::shared_ptr<EmergencyDispatcher> dispatcher,
    std::shared_ptr<SqliteEmergencyRepository> emerRepo,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo) {

    std::cout << "[Test Suite 1/4] Testing Patient Portal SOS, Tracking & History...\n";

    // 1. Register & Login Patient
    auto reg = authService->registerPatient("Ramesh Patient", "+91 98765 43210", "ramesh_p4", "SecurePass123!");
    TEST_ASSERT(reg.success, "Patient registration succeeded");
    TEST_ASSERT(reg.user.role == "PATIENT", "User role is PATIENT");

    auto login = authService->login("ramesh_p4", "SecurePass123!");
    TEST_ASSERT(login.success, "Patient login succeeded");
    std::string token = login.token;

    // Validate token
    auto userOpt = authService->validateToken(token);
    TEST_ASSERT(userOpt.has_value() && userOpt->userId == reg.user.userId, "Session token resolves to registered patient");

    // 2. Patient creates Emergency SOS request
    auto dRes = dispatcher->dispatchEmergency(
        "Ramesh Patient", "+91 98765 43210", "Clock Tower, Dehradun",
        30.3244, 78.0418,
        "Severe Chest Pain / Possible Cardiac Arrest",
        "CRITICAL",
        reg.user.userId);

    TEST_ASSERT(dRes.success, "Emergency SOS dispatched successfully");
    std::string emId = dRes.emergency.emergencyId;
    TEST_ASSERT(!emId.empty(), "Emergency ID generated");
    TEST_ASSERT(dRes.emergency.userId == reg.user.userId, "Emergency linked to patient userId");
    TEST_ASSERT(!dRes.assignedAmbulance.ambulanceId.empty(), "Nearest ambulance unit assigned");
    TEST_ASSERT(!dRes.assignedHospital.hospitalId.empty(), "Specialty hospital assigned");
    TEST_ASSERT(dRes.ambulanceEtaMinutes > 0, "Calculated realistic road ETA");

    // 3. Patient Tracking: Verify emergency lookup and attached telemetry
    auto emOpt = emerRepo->findById(emId);
    TEST_ASSERT(emOpt.has_value(), "Emergency found in SQLite database");
    TEST_ASSERT(emOpt->status == "DISPATCHED", "Initial status is DISPATCHED");

    auto ambOpt = ambRepo->findById(emOpt->assignedAmbulanceId);
    TEST_ASSERT(ambOpt.has_value(), "Assigned ambulance found in SQLite");
    TEST_ASSERT(ambOpt->status == "DISPATCHED", "Ambulance status synchronized to DISPATCHED");
    TEST_ASSERT(ambOpt->assignedEmergencyId == emId, "Ambulance linked to active emergency");

    auto hospOpt = hospRepo->findById(emOpt->assignedHospitalId);
    TEST_ASSERT(hospOpt.has_value(), "Assigned hospital found in SQLite");

    // 4. Patient History Isolation
    auto myEmergencies = emerRepo->findByUserId(reg.user.userId);
    TEST_ASSERT(myEmergencies.size() == 1, "Patient history returns exactly 1 emergency");
    TEST_ASSERT(myEmergencies[0].emergencyId == emId, "Patient history matches created emergency");
}

// =============================================================================
// Test Suite 2: Driver Portal Integration (Tests 2, 7, 8, 9)
// =============================================================================
void testDriverPortalWorkflow(
    std::shared_ptr<AuthService> authService,
    std::shared_ptr<EmergencyDispatcher> dispatcher,
    std::shared_ptr<SqliteEmergencyRepository> emerRepo,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo) {

    (void)hospRepo;
    std::cout << "[Test Suite 2/4] Testing Driver Portal Operations, Telemetry & Lifecycle...\n";

    AuthUser adminUser;
    adminUser.role = "ADMIN";
    adminUser.username = "admin";

    // 1. Provision Driver for AMB-02
    auto dCreate = authService->createDriver(
        adminUser, "driver_karan", "DriverSec123!", "Karan EMT", "+91 98111 22233", "AMB-02");
    TEST_ASSERT(dCreate.success, "Driver account provisioned for AMB-02");

    auto dLogin = authService->login("driver_karan", "DriverSec123!");
    TEST_ASSERT(dLogin.success, "Driver logged in successfully");

    // 2. Driver Availability Management when Standby
    auto ambBefore = ambRepo->findById("AMB-02");
    TEST_ASSERT(ambBefore.has_value() && ambBefore->status == "READY", "AMB-02 initially READY");

    // Toggle to MAINTENANCE
    bool mOk = ambRepo->updateStatus("AMB-02", "MAINTENANCE", "");
    TEST_ASSERT(mOk, "Ambulance status toggled to MAINTENANCE");
    TEST_ASSERT(ambRepo->findById("AMB-02")->status == "MAINTENANCE", "Status is MAINTENANCE in SQLite");

    // Toggle back to READY
    bool rOk = ambRepo->updateStatus("AMB-02", "READY", "");
    TEST_ASSERT(rOk, "Ambulance status toggled back to READY");
    TEST_ASSERT(ambRepo->findById("AMB-02")->status == "READY", "Status is READY in SQLite");

    // 3. Register patient Sunita and dispatch an Emergency
    auto regSunita = authService->registerPatient("Sunita Devi", "+91 98222 33344", "sunita_p4", "SunitaPass123!");
    TEST_ASSERT(regSunita.success, "Patient Sunita registered successfully");
    g_sunitaUserId = regSunita.user.userId;

    auto disp = dispatcher->dispatchEmergency(
        "Sunita Devi", "+91 98222 33344", "Rajpur Road",
        30.3400, 78.0600, "Fracture Trauma", "HIGH", g_sunitaUserId);
    TEST_ASSERT(disp.success, "Emergency dispatch created");
    if (!disp.success) {
        std::cerr << "  [ERROR] Dispatch failed: " << disp.errorCode << " : " << disp.message << "\n";
        return;
    }
    std::string emId = disp.emergency.emergencyId;

    // Ensure AMB-02 is the assigned vehicle (or force assignment for test determinism)
    std::string ambId = disp.assignedAmbulance.ambulanceId;
    TEST_ASSERT(!ambId.empty(), "Assigned ambulance exists");

    // 4. Test Live GPS update
    double newLat = 30.3300;
    double newLng = 78.0500;
    bool posOk = ambRepo->updatePosition(ambId, newLat, newLng);
    TEST_ASSERT(posOk, "Ambulance GPS coordinates updated in SQLite");
    auto ambAfterPos = ambRepo->findById(ambId);
    TEST_ASSERT(ambAfterPos.has_value() && ambAfterPos->currentLat == newLat && ambAfterPos->currentLng == newLng, "GPS coordinates verified in SQLite");

    // 5. Driver Lifecycle Action Progression: DISPATCHED -> ON_SCENE -> TRANSPORTING -> ARRIVED -> COMPLETED
    auto t1 = dispatcher->transitionStatus(emId, "ON_SCENE", "Driver arrived at pickup");
    TEST_ASSERT(t1.success, "Transition to ON_SCENE succeeded");
    TEST_ASSERT(emerRepo->findById(emId)->status == "ON_SCENE", "Emergency status is ON_SCENE in SQLite");
    TEST_ASSERT(ambRepo->findById(ambId)->status == "ON_SCENE", "Ambulance status is ON_SCENE in SQLite");

    auto t2 = dispatcher->transitionStatus(emId, "TRANSPORTING", "Patient boarded, en route to hospital");
    TEST_ASSERT(t2.success, "Transition to TRANSPORTING succeeded");
    TEST_ASSERT(emerRepo->findById(emId)->status == "TRANSPORTING", "Emergency status is TRANSPORTING in SQLite");
    TEST_ASSERT(ambRepo->findById(ambId)->status == "EN_ROUTE", "Ambulance status is EN_ROUTE in SQLite");

    auto t3 = dispatcher->transitionStatus(emId, "ARRIVED", "Arrived at hospital facility");
    TEST_ASSERT(t3.success, "Transition to ARRIVED succeeded");
    TEST_ASSERT(emerRepo->findById(emId)->status == "ARRIVED", "Emergency status is ARRIVED in SQLite");

    auto t4 = dispatcher->transitionStatus(emId, "COMPLETED", "Emergency completed");
    TEST_ASSERT(t4.success, "Transition to COMPLETED succeeded");
    TEST_ASSERT(emerRepo->findById(emId)->status == "COMPLETED", "Emergency status is COMPLETED in SQLite");
    TEST_ASSERT(ambRepo->findById(ambId)->status == "READY", "Ambulance automatically released to READY in SQLite");
    TEST_ASSERT(ambRepo->findById(ambId)->assignedEmergencyId.empty(), "Ambulance assigned emergency cleared");
}

// =============================================================================
// Test Suite 3: Hospital Portal Integration (Tests 3, 10, 11)
// =============================================================================
void testHospitalPortalWorkflow(
    std::shared_ptr<AuthService> authService,
    std::shared_ptr<EmergencyDispatcher> dispatcher,
    std::shared_ptr<SqliteEmergencyRepository> emerRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo) {

    std::cout << "[Test Suite 3/4] Testing Hospital Portal Bed Inventory & Admissions...\n";

    AuthUser adminUser;
    adminUser.role = "ADMIN";
    adminUser.username = "admin";

    // 1. Provision Hospital Account for LOC-004
    auto hCreate = authService->createHospital(
        adminUser, "max_hosp_admin", "HospPass123!", "Max Operations", "+91 135 7120000", "LOC-004");
    TEST_ASSERT(hCreate.success, "Hospital account provisioned for LOC-004");

    auto hLogin = authService->login("max_hosp_admin", "HospPass123!");
    TEST_ASSERT(hLogin.success, "Hospital admin logged in successfully");

    // 2. Query Bed Profile
    auto hospBefore = hospRepo->findById("LOC-004");
    TEST_ASSERT(hospBefore.has_value(), "Hospital record exists in SQLite");

    // 3. Update Bed Inventory
    int newTotal = 150;
    int newIcuTotal = 30;
    int newIcuAvail = 22;
    int newOxygen = 40;

    bool bedsOk = hospRepo->updateBeds("LOC-004", newTotal, newIcuTotal, newIcuAvail, newOxygen);
    TEST_ASSERT(bedsOk, "Hospital beds updated in SQLite");

    auto hospAfter = hospRepo->findById("LOC-004");
    TEST_ASSERT(hospAfter->totalBeds == newTotal, "Total beds match updated count");
    TEST_ASSERT(hospAfter->icuBeds == newIcuTotal, "Total ICU beds match updated count");
    TEST_ASSERT(hospAfter->availableIcu == newIcuAvail, "Available ICU beds match updated count");
    TEST_ASSERT(hospAfter->oxygenBeds == newOxygen, "Oxygen beds match updated count");

    // 4. Incoming Emergency Queue & Admission Confirmation
    auto regRajesh = authService->registerPatient("Rajesh Gupta", "+91 98333 44455", "rajesh_p4", "RajeshPass123!");
    TEST_ASSERT(regRajesh.success, "Patient Rajesh registered successfully");
    g_rajeshUserId = regRajesh.user.userId;

    auto disp = dispatcher->dispatchEmergency(
        "Rajesh Gupta", "+91 98333 44455", "ISBT Dehradun",
        30.2884, 78.0069, "Breathing Distress / Low O2", "HIGH", g_rajeshUserId);
    TEST_ASSERT(disp.success, "Incoming emergency dispatched");
    if (!disp.success) {
        std::cerr << "  [ERROR] Hospital incoming dispatch failed: " << disp.errorCode << " : " << disp.message << "\n";
        return;
    }
    std::string emId = disp.emergency.emergencyId;

    // Verify hospital sees this emergency in its queue
    auto incoming = emerRepo->findByHospitalId(disp.assignedHospital.hospitalId);
    bool foundInQueue = false;
    for (const auto& e : incoming) {
        if (e.emergencyId == emId) {
            foundInQueue = true;
            break;
        }
    }
    TEST_ASSERT(foundInQueue, "Emergency found in hospital incoming queue");

    // Hospital confirms admission: transitions to COMPLETED
    dispatcher->transitionStatus(emId, "ON_SCENE", "Ambulance on scene");
    dispatcher->transitionStatus(emId, "TRANSPORTING", "Ambulance transporting");
    dispatcher->transitionStatus(emId, "ARRIVED", "Arrived at hospital");
    auto admitRes = dispatcher->transitionStatus(emId, "COMPLETED", "Hospital admission confirmed");
    TEST_ASSERT(admitRes.success, "Hospital admission successfully completes emergency");
    TEST_ASSERT(emerRepo->findById(emId)->status == "COMPLETED", "Emergency status updated to COMPLETED");
}

// =============================================================================
// Test Suite 4: Cross-Role Security & Database Metrics (Tests 12, 13, 14, 15)
// =============================================================================
void testCrossRoleSecurityAndConsistency(
    std::shared_ptr<AuthService> authService,
    std::shared_ptr<SqliteUserRepository> userRepo,
    std::shared_ptr<SqliteEmergencyRepository> emerRepo,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo) {

    std::cout << "[Test Suite 4/4] Testing Cross-Role Security, Ownership & Consistency...\n";

    // 1. Patient cannot perform admin operations
    AuthUser patientUser;
    patientUser.role = "PATIENT";
    patientUser.username = "test_patient";
    patientUser.userId = 999;

    auto fakeDriver = authService->createDriver(
        patientUser, "rogue_driver", "Pass123!", "Rogue", "123", "AMB-01");
    TEST_ASSERT(!fakeDriver.success, "Patient role is blocked from provisioning drivers (HTTP 403)");

    auto fakeHosp = authService->createHospital(
        patientUser, "rogue_hosp", "Pass123!", "Rogue Hosp", "123", "LOC-004");
    TEST_ASSERT(!fakeHosp.success, "Patient role is blocked from provisioning hospitals (HTTP 403)");

    // 2. Resource Ownership Isolation
    auto p1 = emerRepo->findByUserId(g_sunitaUserId);
    auto p2 = emerRepo->findByUserId(g_rajeshUserId);
    TEST_ASSERT(!p1.empty(), "User Sunita has emergencies");
    TEST_ASSERT(!p2.empty(), "User Rajesh has emergencies");
    TEST_ASSERT(p1[0].emergencyId != p2[0].emergencyId, "User Sunita and Rajesh have strictly isolated records");

    // 3. Database Consistency Across All Portals
    size_t totalUsers = userRepo->findAll().size();
    TEST_ASSERT(totalUsers >= 4, "Users table contains seeded admin, patients, drivers, and hospitals");

    size_t totalAmbs = ambRepo->count();
    TEST_ASSERT(totalAmbs >= 4, "Ambulances table contains active vehicles (4 units seeded)");

    size_t totalHosps = hospRepo->count();
    TEST_ASSERT(totalHosps >= 6, "Hospitals table contains active facilities (6 hospitals seeded)");

    size_t totalEmergencies = emerRepo->count();
    TEST_ASSERT(totalEmergencies >= 3, "Emergencies recorded persistently in SQLite");
}

// =============================================================================
// Main Test Runner
// =============================================================================
int main() {
    std::cout << "========================================================================\n";
    std::cout << "  JEEVAN MARG -- PHASE 4 MULTI-USER DASHBOARD INTEGRATION TEST RUNNER\n";
    std::cout << "  Testing Patient, Driver, Hospital Portals & Cross-Role Synchronization\n";
    std::cout << "========================================================================\n\n";

    std::string seedDir = "seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "m3_navigation/seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "../seed";

    std::string schemaFile = locateSchema();

    // 1. Setup in-memory SQLite database
    Database db;
    db.open(":memory:");
    db.execFile(schemaFile);
    DatabaseBootstrap::initializeAndSeed(db, ":memory:", seedDir);

    // 2. Repositories
    auto locRepo = std::make_shared<SqliteLocationRepository>(db);
    auto roadRepo = std::make_shared<SqliteRoadRepository>(db);
    auto hospRepo = std::make_shared<SqliteHospitalRepository>(db);
    auto ambRepo = std::make_shared<SqliteAmbulanceRepository>(db);
    auto emerRepo = std::make_shared<SqliteEmergencyRepository>(db);
    auto patRepo = std::make_shared<SqlitePatientLocationRepository>(db);
    auto userRepo = std::make_shared<SqliteUserRepository>(db);
    auto sessionRepo = std::make_shared<SqliteSessionRepository>(db);

    // 3. Graph, Pathfinding & Dispatcher
    auto edges = roadRepo->findAll();
    auto graph = RoadGraphBuilder::buildGraph(*locRepo, edges, true);
    auto pathFinder = std::make_shared<ShortestPathFinder>(graph);
    auto estimator = std::make_shared<TransportEstimator>();
    auto dispatcher = std::make_shared<EmergencyDispatcher>(
        db, emerRepo, ambRepo, hospRepo, graph, pathFinder, estimator);

    auto authService = std::make_shared<AuthService>(userRepo, sessionRepo, ambRepo, hospRepo);

    // 4. Run Test Suites
    testPatientPortalWorkflow(authService, dispatcher, emerRepo, ambRepo, hospRepo);
    testDriverPortalWorkflow(authService, dispatcher, emerRepo, ambRepo, hospRepo);
    testHospitalPortalWorkflow(authService, dispatcher, emerRepo, hospRepo);
    testCrossRoleSecurityAndConsistency(authService, userRepo, emerRepo, ambRepo, hospRepo);

    std::cout << "\n========================================================================\n";
    std::cout << "  PHASE 4 PORTALS TEST SUMMARY: " << g_testsPassed << " / " << g_testsRun << " ASSERTIONS PASSED (";
    if (g_testsPassed == g_testsRun) {
        std::cout << "100% SUCCESS - ALL PHASE 4 PORTAL TESTS GREEN!)\n";
    } else {
        std::cout << (g_testsRun - g_testsPassed) << " FAILED)\n";
    }
    std::cout << "========================================================================\n\n";

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
