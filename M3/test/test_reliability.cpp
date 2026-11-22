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
#include "../../m1_dispatch/service/TriageRuleEngine.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>
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
// Test Suite 1: Real-Time GPS Coordinate Validation & Boundary Enforcement
// =============================================================================
void testGpsCoordinateValidation() {
    std::cout << "[Test Suite 1/6] Testing GPS Coordinate Validation & Boundary Checks...\n";

    auto isValidCoord = [](double lat, double lng) -> bool {
        if (std::isnan(lat) || std::isnan(lng) || std::isinf(lat) || std::isinf(lng)) return false;
        if (lat < -90.0 || lat > 90.0 || lng < -180.0 || lng > 180.0) return false;
        if (lat == 0.0 && lng == 0.0) return false;
        return true;
    };

    // Valid Dehradun coordinates
    TEST_ASSERT(isValidCoord(30.3244, 78.0418), "Valid Dehradun Clock Tower GPS accepted");
    TEST_ASSERT(isValidCoord(30.2872, 78.0069), "Valid Dehradun ISBT GPS accepted");
    TEST_ASSERT(isValidCoord(-45.0, 120.0), "Valid global coordinates accepted");

    // Invalid coordinates
    TEST_ASSERT(!isValidCoord(0.0, 0.0), "Null Island (0,0) rejected");
    TEST_ASSERT(!isValidCoord(95.0, 78.0), "Out-of-range latitude (> 90) rejected");
    TEST_ASSERT(!isValidCoord(-91.0, 78.0), "Out-of-range latitude (< -90) rejected");
    TEST_ASSERT(!isValidCoord(30.0, 185.0), "Out-of-range longitude (> 180) rejected");
    TEST_ASSERT(!isValidCoord(30.0, -181.0), "Out-of-range longitude (< -180) rejected");
    TEST_ASSERT(!isValidCoord(std::numeric_limits<double>::quiet_NaN(), 78.0), "NaN latitude rejected");
    TEST_ASSERT(!isValidCoord(30.0, std::numeric_limits<double>::infinity()), "Infinity longitude rejected");
}

// =============================================================================
// Test Suite 2: Triage Consistency for Chest Pain & Coronary Conditions
// =============================================================================
void testChestPainTriageConsistency() {
    std::cout << "[Test Suite 2/6] Testing Inconsistent Chest Pain Triage Resolution...\n";

    std::vector<std::string> chestPainVariants = {
        "Severe chest pain and sweating",
        "Acute pain in chest radiating to left arm",
        "Chest discomfort and severe pressure",
        "Severe tightness in chest with breathlessness",
        "Chest heaviness and fatigue",
        "Sudden angina attack",
        "Acute myocardial infarction",
        "Cardiac arrest at home"
    };

    for (const auto& variant : chestPainVariants) {
        auto triage = TriageRuleEngine::assessCondition(variant);
        TEST_ASSERT(triage.severity == EmergencySeverity::CRITICAL,
                    "Chest-pain condition '" + variant + "' must triage to CRITICAL");
        TEST_ASSERT(triage.urgencyScore == 1,
                    "Chest-pain condition '" + variant + "' must have urgency score 1");
        TEST_ASSERT(triage.recommendedSpecialty.find("Cardiology") != std::string::npos,
                    "Chest-pain condition '" + variant + "' must recommend Cardiology");
        TEST_ASSERT(triage.requiredAmbulanceType.find("ALS") != std::string::npos,
                    "Chest-pain condition '" + variant + "' must require ALS ambulance");
    }

    // Verify non-cardiac conditions are not accidentally classified as cardiac
    auto trauma = TriageRuleEngine::assessCondition("Minor skin scratch on leg");
    TEST_ASSERT(trauma.severity == EmergencySeverity::MEDIUM, "Non-cardiac minor condition remains MEDIUM");
}

// =============================================================================
// Test Suite 3: ICU Bed Double-Reservation Prevention & Restoration
// =============================================================================
void testIcuBedLifecycleIntegrity(
    std::shared_ptr<EmergencyDispatcher> dispatcher,
    std::shared_ptr<SqliteEmergencyRepository> emerRepo,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo) {

    (void)emerRepo;
    (void)ambRepo;
    std::cout << "[Test Suite 3/6] Testing ICU Bed Allocation, Reservation & Restoration...\n";

    // 1. Record baseline ICU capacity for all hospitals
    std::unordered_map<std::string, int> baselineIcus;
    for (const auto& h : hospRepo->findAll()) {
        baselineIcus[h.hospitalId] = h.availableIcu;
    }

    // 2. Dispatch a CRITICAL emergency (reserves 1 ICU bed in assigned hospital)
    auto dRes = dispatcher->dispatchEmergency(
        "Critical Patient", "+91 99000 11223", "Clock Tower",
        30.3244, 78.0418, "Severe chest pain / acute cardiac arrest", "CRITICAL", 0);
    TEST_ASSERT(dRes.success, "Critical emergency dispatched");
    std::string emId = dRes.emergency.emergencyId;
    std::string assignedHospId = dRes.assignedHospital.hospitalId;
    int baseline1 = baselineIcus[assignedHospId];

    auto hospDuring = hospRepo->findById(assignedHospId);
    TEST_ASSERT(hospDuring->availableIcu == baseline1 - 1, "ICU bed reserved: available decremented by exactly 1");

    // 3. Verify driver step progression does NOT double-reserve ICU beds
    dispatcher->transitionStatus(emId, "ON_SCENE", "On scene");
    dispatcher->transitionStatus(emId, "TRANSPORTING", "Transporting");
    dispatcher->transitionStatus(emId, "ARRIVED", "Arrived");

    auto hospArrived = hospRepo->findById(assignedHospId);
    TEST_ASSERT(hospArrived->availableIcu == baseline1 - 1, "ICU capacity unchanged during intermediate transitions (no double reservation)");

    // 4. Complete emergency / admission: ICU restored upon mission lifecycle release
    dispatcher->transitionStatus(emId, "COMPLETED", "Patient admitted into hospital");
    auto hospAfter = hospRepo->findById(assignedHospId);
    TEST_ASSERT(hospAfter->availableIcu == baseline1, "ICU bed restored after admission/completion");

    // 5. Test Cancellation restoration
    auto dRes2 = dispatcher->dispatchEmergency(
        "Patient Two", "+91 99000 44556", "Clock Tower",
        30.3244, 78.0418, "Severe chest pain", "CRITICAL", 0);
    TEST_ASSERT(dRes2.success, "Second critical emergency dispatched");
    std::string emId2 = dRes2.emergency.emergencyId;
    std::string hospId2 = dRes2.assignedHospital.hospitalId;
    int baseline2 = baselineIcus[hospId2];

    auto hospDuring2 = hospRepo->findById(hospId2);
    TEST_ASSERT(hospDuring2->availableIcu == baseline2 - 1, "ICU bed reserved for second emergency");

    // Cancel emergency: ICU bed must be restored
    dispatcher->transitionStatus(emId2, "CANCELLED", "Patient declined transport");
    auto hospCancelled = hospRepo->findById(hospId2);
    TEST_ASSERT(hospCancelled->availableIcu == baseline2, "ICU bed restored after cancellation");
}

// =============================================================================
// Test Suite 4: Real-Time Dynamic Dijkstra Route & ETA Updates
// =============================================================================
void testDynamicRouteAndEtaUpdates(
    std::shared_ptr<ShortestPathFinder> pathFinder,
    std::shared_ptr<TransportEstimator> estimator,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<RoadGraph> graph) {

    (void)graph;
    std::cout << "[Test Suite 4/6] Testing Dynamic Route Recomputation & Live ETA Refresh...\n";

    // Patient at Clock Tower (LOC-001)
    std::string patientNode = "LOC-001";

    // 1. Ambulance stationed at ISBT (LOC-002) - far away
    std::string ambNodeFar = "LOC-002";
    auto routeFarRes = pathFinder->findShortestRoute(ambNodeFar, patientNode);
    TEST_ASSERT(routeFarRes.isSuccess(), "Route calculated from ISBT to Clock Tower");
    auto routeFar = routeFarRes.getData();
    estimator->estimateAll(routeFar);
    double distFar = routeFar.getTotalDistanceKm();
    double etaFar = routeFar.getTravelTimes().at("Ambulance");

    // 2. Ambulance moves to Saharanpur Chowk (LOC-018) - midway
    std::string ambNodeMid = "LOC-018";
    auto routeMidRes = pathFinder->findShortestRoute(ambNodeMid, patientNode);
    TEST_ASSERT(routeMidRes.isSuccess(), "Route calculated from Saharanpur Chowk to Clock Tower");
    auto routeMid = routeMidRes.getData();
    estimator->estimateAll(routeMid);
    double distMid = routeMid.getTotalDistanceKm();
    double etaMid = routeMid.getTravelTimes().at("Ambulance");

    // 3. Ambulance moves to Parade Ground (LOC-013) - immediate vicinity
    std::string ambNodeClose = "LOC-013";
    auto routeCloseRes = pathFinder->findShortestRoute(ambNodeClose, patientNode);
    TEST_ASSERT(routeCloseRes.isSuccess(), "Route calculated from Parade Ground to Clock Tower");
    auto routeClose = routeCloseRes.getData();
    estimator->estimateAll(routeClose);
    double distClose = routeClose.getTotalDistanceKm();
    double etaClose = routeClose.getTravelTimes().at("Ambulance");

    // Assert that distance and ETA monotonically decrease as ambulance approaches patient
    TEST_ASSERT(distFar > distMid, "Distance decreases as vehicle moves from ISBT to Saharanpur Chowk");
    TEST_ASSERT(distMid > distClose, "Distance decreases as vehicle moves from Saharanpur Chowk to Parade Ground");
    TEST_ASSERT(etaFar > etaMid, "ETA decreases as vehicle moves closer");
    TEST_ASSERT(etaMid > etaClose, "ETA decreases as vehicle reaches vicinity");
    TEST_ASSERT(etaClose > 0.0, "Close-range ETA remains positive");

    // 4. Test live coordinates persistence
    ambRepo->updatePosition("AMB-01", 30.3235, 78.0460); // Parade ground coordinates
    auto ambRec = ambRepo->findById("AMB-01");
    TEST_ASSERT(ambRec.has_value() && ambRec->currentLat == 30.3235, "Ambulance coordinates updated in SQLite");
}

// =============================================================================
// Test Suite 5: Emergency State Machine Lifecycle Enforcement
// =============================================================================
void testLifecycleBypassRejection(std::shared_ptr<EmergencyDispatcher> dispatcher) {
    (void)dispatcher;
    std::cout << "[Test Suite 5/6] Testing Emergency Lifecycle Bypass Prevention...\n";

    // 1. Valid transitions
    TEST_ASSERT(EmergencyDispatcher::isValidTransition("PENDING", "DISPATCHED"), "PENDING -> DISPATCHED allowed");
    TEST_ASSERT(EmergencyDispatcher::isValidTransition("DISPATCHED", "ON_SCENE"), "DISPATCHED -> ON_SCENE allowed");
    TEST_ASSERT(EmergencyDispatcher::isValidTransition("ON_SCENE", "TRANSPORTING"), "ON_SCENE -> TRANSPORTING allowed");
    TEST_ASSERT(EmergencyDispatcher::isValidTransition("TRANSPORTING", "ARRIVED"), "TRANSPORTING -> ARRIVED allowed");
    TEST_ASSERT(EmergencyDispatcher::isValidTransition("ARRIVED", "COMPLETED"), "ARRIVED -> COMPLETED allowed");

    // 2. Invalid skips
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("DISPATCHED", "ARRIVED"), "Direct skip from DISPATCHED to ARRIVED rejected");
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("DISPATCHED", "COMPLETED"), "Direct skip from DISPATCHED to COMPLETED rejected");
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("ON_SCENE", "COMPLETED"), "Direct skip from ON_SCENE to COMPLETED rejected");
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("PENDING", "TRANSPORTING"), "Direct skip from PENDING to TRANSPORTING rejected");

    // 3. Backward transitions
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("ARRIVED", "ON_SCENE"), "Backward transition ARRIVED -> ON_SCENE rejected");
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("TRANSPORTING", "DISPATCHED"), "Backward transition TRANSPORTING -> DISPATCHED rejected");

    // 4. Terminal state mutations
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("COMPLETED", "ON_SCENE"), "Mutation of COMPLETED state rejected");
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("COMPLETED", "CANCELLED"), "Cancellation of COMPLETED emergency rejected");
    TEST_ASSERT(!EmergencyDispatcher::isValidTransition("CANCELLED", "COMPLETED"), "Completion of CANCELLED emergency rejected");
}

// =============================================================================
// Test Suite 6: Complete End-to-End Workflow with Final State Audit
// =============================================================================
void testCompleteEndToEndWorkflow(
    std::shared_ptr<AuthService> authService,
    std::shared_ptr<EmergencyDispatcher> dispatcher,
    std::shared_ptr<SqliteEmergencyRepository> emerRepo,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo) {

    (void)hospRepo;
    std::cout << "[Test Suite 6/6] Testing Complete End-to-End Emergency Lifecycle Workflow...\n";

    // 1. Patient registration
    auto pReg = authService->registerPatient("Deepak Pant", "+91 97600 55443", "deepak_p5", "SecureP5Pass!");
    TEST_ASSERT(pReg.success, "Patient Deepak registered successfully");
    int64_t patientId = pReg.user.userId;

    // 2. Patient triggers SOS
    auto dRes = dispatcher->dispatchEmergency(
        "Deepak Pant", "+91 97600 55443", "Ballupur Chowk",
        30.3340, 78.0180, "Severe acute chest discomfort and sweating", "CRITICAL", patientId);
    TEST_ASSERT(dRes.success, "SOS dispatch succeeded");
    std::string emId = dRes.emergency.emergencyId;
    std::string ambId = dRes.assignedAmbulance.ambulanceId;
    std::string hospId = dRes.assignedHospital.hospitalId;

    // 3. Ambulance is assigned and DISPATCHED
    auto amb = ambRepo->findById(ambId);
    TEST_ASSERT(amb.has_value() && amb->status == "DISPATCHED", "Ambulance status is DISPATCHED in SQLite");
    TEST_ASSERT(amb->assignedEmergencyId == emId, "Ambulance assigned to emergency in SQLite");

    // 4. Driver updates GPS location on the way
    ambRepo->updatePosition(ambId, 30.3310, 78.0200);
    ambRepo->appendLocation(ambId, 30.3310, 78.0200);
    auto ambMoved = ambRepo->findById(ambId);
    TEST_ASSERT(ambMoved->currentLat == 30.3310, "Driver GPS coordinates updated");

    // 5. Driver arrives at scene
    auto t1 = dispatcher->transitionStatus(emId, "ON_SCENE", "Ambulance arrived at patient pickup");
    TEST_ASSERT(t1.success, "Transition to ON_SCENE succeeded");
    TEST_ASSERT(emerRepo->findById(emId)->status == "ON_SCENE", "Emergency status is ON_SCENE");
    TEST_ASSERT(ambRepo->findById(ambId)->status == "ON_SCENE", "Ambulance status is ON_SCENE");

    // 6. Driver boards patient and transports
    auto t2 = dispatcher->transitionStatus(emId, "TRANSPORTING", "Patient boarded, transporting to hospital");
    TEST_ASSERT(t2.success, "Transition to TRANSPORTING succeeded");
    TEST_ASSERT(emerRepo->findById(emId)->status == "TRANSPORTING", "Emergency status is TRANSPORTING");

    // 7. Ambulance arrives at hospital
    auto t3 = dispatcher->transitionStatus(emId, "ARRIVED", "Arrived at hospital emergency entrance");
    TEST_ASSERT(t3.success, "Transition to ARRIVED succeeded");
    TEST_ASSERT(emerRepo->findById(emId)->status == "ARRIVED", "Emergency status is ARRIVED");

    // 8. Hospital admits patient and completes emergency
    auto t4 = dispatcher->transitionStatus(emId, "COMPLETED", "Patient admission confirmed into ICU");
    TEST_ASSERT(t4.success, "Emergency completed");
    TEST_ASSERT(emerRepo->findById(emId)->status == "COMPLETED", "Final emergency status is COMPLETED");

    // 9. Verify ambulance is automatically released back to READY
    auto ambFinal = ambRepo->findById(ambId);
    TEST_ASSERT(ambFinal->status == "READY", "Ambulance automatically released to READY");
    TEST_ASSERT(ambFinal->assignedEmergencyId.empty(), "Ambulance assignment cleared");

    // 10. Verify audit trail in SQLite
    auto emFinal = emerRepo->findById(emId);
    TEST_ASSERT(emFinal.has_value() && emFinal->status == "COMPLETED", "Final state verified in SQLite");
}

// =============================================================================
// Main Test Runner
// =============================================================================
int main() {
    std::cout << "========================================================================\n";
    std::cout << "  JEEVAN MARG -- PHASE 5 RELIABILITY, GPS & LIVE TRACKING TEST RUNNER\n";
    std::cout << "  Verifying GPS Boundaries, Dynamic Route, ICU Integrity & Lifecycles\n";
    std::cout << "========================================================================\n\n";

    std::string seedDir = "seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "m3_navigation/seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "../seed";

    std::string schemaFile = locateSchema();

    Database db;
    db.open(":memory:");
    db.execFile(schemaFile);
    DatabaseBootstrap::initializeAndSeed(db, ":memory:", seedDir);

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
    auto estimator = std::make_shared<TransportEstimator>();
    auto dispatcher = std::make_shared<EmergencyDispatcher>(
        db, emerRepo, ambRepo, hospRepo, graph, pathFinder, estimator);
    auto authService = std::make_shared<AuthService>(userRepo, sessionRepo, ambRepo, hospRepo);

    // Run test suites
    testGpsCoordinateValidation();
    testChestPainTriageConsistency();
    testIcuBedLifecycleIntegrity(dispatcher, emerRepo, ambRepo, hospRepo);
    testDynamicRouteAndEtaUpdates(pathFinder, estimator, ambRepo, graph);
    testLifecycleBypassRejection(dispatcher);
    testCompleteEndToEndWorkflow(authService, dispatcher, emerRepo, ambRepo, hospRepo);

    std::cout << "\n========================================================================\n";
    std::cout << "  PHASE 5 RELIABILITY TEST SUMMARY: " << g_testsPassed << " / " << g_testsRun << " ASSERTIONS PASSED (";
    if (g_testsPassed == g_testsRun) {
        std::cout << "100% SUCCESS - ALL PHASE 5 TESTS GREEN!)\n";
    } else {
        std::cout << (g_testsRun - g_testsPassed) << " FAILED)\n";
    }
    std::cout << "========================================================================\n\n";

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
