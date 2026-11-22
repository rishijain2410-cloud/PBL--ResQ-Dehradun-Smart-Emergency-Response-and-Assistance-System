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
#include <thread>
#include <atomic>

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

// -----------------------------------------------------------------------------
// Test Suite 1: Crypto, Password Hashing & Token Management (Tests 5, 6, 7, 8, 20)
// -----------------------------------------------------------------------------
void testCryptoAndTokens(std::shared_ptr<SqliteUserRepository> userRepo,
                         std::shared_ptr<SqliteSessionRepository> sessionRepo,
                         std::shared_ptr<AuthService> authService) {
    (void)userRepo;
    (void)sessionRepo;
    std::cout << "[Test Suite 1/5] Testing Cryptography, Password Hashing & Tokens...\n";

    // 1. Password hashing format: pbkdf2_sha256$100000$<salt>$<hash>
    std::string pass = "SecurePass123!";
    std::string hash1 = CryptoUtil::hashPassword(pass);
    std::string hash2 = CryptoUtil::hashPassword(pass);

    TEST_ASSERT(hash1.rfind("pbkdf2_sha256$100000$", 0) == 0, "Hash must use PBKDF2-HMAC-SHA256 with 100k iterations");
    TEST_ASSERT(hash1 != pass, "Password must never be stored as plaintext");
    TEST_ASSERT(hash1 != hash2, "Each password hash must have a unique cryptographic salt");
    TEST_ASSERT(CryptoUtil::verifyPassword(pass, hash1), "Password verification succeeds with correct password");
    TEST_ASSERT(!CryptoUtil::verifyPassword("WrongPassword!", hash1), "Password verification fails with incorrect password");

    // 2. Token generation & SHA-256 hashing
    std::string token1 = CryptoUtil::generateToken(32);
    std::string token2 = CryptoUtil::generateToken(32);
    TEST_ASSERT(token1.size() == 64, "Generated session token must be 64 hex characters (32 bytes)");
    TEST_ASSERT(token1 != token2, "Session tokens must be distinct and cryptographically random");

    std::string tokenHash = CryptoUtil::sha256Hex(token1);
    TEST_ASSERT(tokenHash.size() == 64, "Token hash must be a 64-char SHA-256 hex string");
    TEST_ASSERT(tokenHash != token1, "Token hash must not match the raw token");

    // 3. Constant time string comparison
    TEST_ASSERT(CryptoUtil::constantTimeCompare("abcdef", "abcdef"), "Constant-time compare matches equal strings");
    TEST_ASSERT(!CryptoUtil::constantTimeCompare("abcdef", "abcdeg"), "Constant-time compare rejects unequal strings");
    TEST_ASSERT(!CryptoUtil::constantTimeCompare("abcdef", "abc"), "Constant-time compare rejects different lengths");

    // 4. Token expiration & revocation via session repository
    // Register temporary user
    auto reg = authService->registerPatient("Token Test", "+91 99999 11111", "token_user", "TokenPass123!");
    TEST_ASSERT(reg.success, "User creation for token testing succeeded");

    // Valid token validation
    auto authUser = authService->validateToken(reg.token);
    TEST_ASSERT(authUser.has_value(), "Valid token resolves to active user");
    TEST_ASSERT(authUser->username == "token_user", "Resolved username matches");

    // Revocation (Logout)
    bool loggedOut = authService->logout(reg.token);
    TEST_ASSERT(loggedOut, "Logout succeeds");
    auto revokedUser = authService->validateToken(reg.token);
    TEST_ASSERT(!revokedUser.has_value(), "Revoked token must be rejected upon validation");

    // Malformed / invalid tokens
    TEST_ASSERT(!authService->validateToken("").has_value(), "Empty token rejected");
    TEST_ASSERT(!authService->validateToken("malformed_fake_token_12345").has_value(), "Forged token rejected");
}

// -----------------------------------------------------------------------------
// Test Suite 2: Patient Registration & Login (Tests 1, 2, 3, 4, 9, 12, 13, 14, 19)
// -----------------------------------------------------------------------------
void testPatientRegistrationAndLogin(std::shared_ptr<SqliteUserRepository> userRepo,
                                     std::shared_ptr<AuthService> authService) {
    (void)userRepo;
    std::cout << "[Test Suite 2/5] Testing Patient Registration, Login & Validation...\n";

    // 1. Successful patient registration
    auto reg = authService->registerPatient("Ravi Verma", "+91 98765 00001", "ravi_patient", "RaviSecret2026!");
    TEST_ASSERT(reg.success, "Patient registration succeeds");
    TEST_ASSERT(reg.user.role == "PATIENT", "Registered user role must strictly be PATIENT");
    TEST_ASSERT(!reg.token.empty(), "Registration returns a session token");

    // 2. Duplicate username rejection
    auto dup = authService->registerPatient("Another Ravi", "+91 98765 00002", "ravi_patient", "AnotherSecret2026!");
    TEST_ASSERT(!dup.success, "Duplicate username must be rejected");
    TEST_ASSERT(dup.errorCode == "DUPLICATE_USERNAME", "Error code must be DUPLICATE_USERNAME");

    // 3. Validation: short password rejected
    auto shortPass = authService->registerPatient("Short Pass", "+91 98765 00003", "short_user", "short");
    TEST_ASSERT(!shortPass.success, "Password < 8 characters must be rejected");

    // 4. Validation: invalid username rejected
    auto badUser = authService->registerPatient("Bad User", "+91 98765 00004", "usr!", "ValidPassword123!");
    TEST_ASSERT(!badUser.success, "Username with special characters like '!' must be rejected");

    // 5. Successful login
    auto loginOk = authService->login("ravi_patient", "RaviSecret2026!");
    TEST_ASSERT(loginOk.success, "Login with correct password succeeds");
    TEST_ASSERT(!loginOk.token.empty(), "Login returns session token");
    TEST_ASSERT(loginOk.user.username == "ravi_patient", "Login profile has correct username");

    // 6. Incorrect password rejection
    auto loginFail = authService->login("ravi_patient", "WrongPassword!");
    TEST_ASSERT(!loginFail.success, "Login with incorrect password fails");
    TEST_ASSERT(loginFail.errorCode == "INVALID_CREDENTIALS", "Error code must be INVALID_CREDENTIALS");

    // 7. Non-existent username rejection
    auto loginNoUser = authService->login("ghost_user_999", "SomePassword!");
    TEST_ASSERT(!loginNoUser.success, "Login with non-existent user fails");
    TEST_ASSERT(loginNoUser.errorCode == "INVALID_CREDENTIALS", "Error code must be INVALID_CREDENTIALS");

    // 8. Account deactivation prevents authentication
    AuthUser adminUser;
    adminUser.role = "ADMIN";
    adminUser.username = "admin";

    authService->setUserActive(adminUser, reg.user.userId, false);
    auto disabledLogin = authService->login("ravi_patient", "RaviSecret2026!");
    TEST_ASSERT(!disabledLogin.success, "Deactivated account cannot log in");
    TEST_ASSERT(disabledLogin.errorCode == "ACCOUNT_DISABLED", "Error code must be ACCOUNT_DISABLED");

    // Re-activate for further tests
    authService->setUserActive(adminUser, reg.user.userId, true);
    auto reactivatedLogin = authService->login("ravi_patient", "RaviSecret2026!");
    TEST_ASSERT(reactivatedLogin.success, "Re-activated account can log in again");

    // 9. SQL injection resilience
    auto sqli = authService->login("' OR '1'='1", "' OR '1'='1");
    TEST_ASSERT(!sqli.success, "SQL injection login attempt must fail safely");
}

// -----------------------------------------------------------------------------
// Test Suite 3: Admin Account Provisioning (Tests 10, 11, 18)
// -----------------------------------------------------------------------------
void testAdminProvisioning(std::shared_ptr<AuthService> authService) {
    std::cout << "[Test Suite 3/5] Testing Admin Driver & Hospital Provisioning...\n";

    AuthUser adminUser;
    adminUser.role = "ADMIN";
    adminUser.username = "admin";

    AuthUser patientUser;
    patientUser.role = "PATIENT";
    patientUser.username = "impostor";

    // 1. Non-admin attempting to create a driver is rejected
    auto nonAdminDriver = authService->createDriver(
        patientUser, "driver_rogue", "DriverPass123!", "Rogue Driver", "+91 98970 00001", "AMB-01");
    TEST_ASSERT(!nonAdminDriver.success, "Non-admin cannot create driver accounts");
    TEST_ASSERT(nonAdminDriver.errorCode == "FORBIDDEN", "Error code must be FORBIDDEN");

    // 2. Admin creates driver linked to AMB-01
    auto driverOk = authService->createDriver(
        adminUser, "driver_amb01", "DriverPass123!", "Vikram Driver", "+91 98970 11001", "AMB-01");
    TEST_ASSERT(driverOk.success, "Admin can create driver account linked to AMB-01");
    TEST_ASSERT(driverOk.user.role == "DRIVER", "Driver account has DRIVER role");
    TEST_ASSERT(driverOk.user.linkedAmbulanceId == "AMB-01", "Driver account is linked to AMB-01");

    // 3. Prevent duplicate driver-to-ambulance assignment
    auto dupAmbDriver = authService->createDriver(
        adminUser, "driver_amb01_dup", "DriverPass123!", "Second Driver", "+91 98970 11099", "AMB-01");
    TEST_ASSERT(!dupAmbDriver.success, "Cannot assign multiple active drivers to the same ambulance");
    TEST_ASSERT(dupAmbDriver.errorCode == "AMBULANCE_ALREADY_ASSIGNED", "Error code must be AMBULANCE_ALREADY_ASSIGNED");

    // 4. Admin creates hospital linked to LOC-004 (Max Super Speciality)
    auto hospOk = authService->createHospital(
        adminUser, "hospital_max", "HospPass123!", "Max Hospital Admin", "+91 135 7120000", "LOC-004");
    TEST_ASSERT(hospOk.success, "Admin can create hospital account linked to LOC-004");
    TEST_ASSERT(hospOk.user.role == "HOSPITAL", "Hospital account has HOSPITAL role");
    TEST_ASSERT(hospOk.user.linkedHospitalId == "LOC-004", "Hospital account is linked to LOC-004");

    // 5. Driver can log in
    auto driverLogin = authService->login("driver_amb01", "DriverPass123!");
    TEST_ASSERT(driverLogin.success, "Driver can log in successfully");
    TEST_ASSERT(driverLogin.user.role == "DRIVER", "Logged-in driver has role DRIVER");

    // 6. Hospital can log in
    auto hospLogin = authService->login("hospital_max", "HospPass123!");
    TEST_ASSERT(hospLogin.success, "Hospital can log in successfully");
    TEST_ASSERT(hospLogin.user.role == "HOSPITAL", "Logged-in hospital has role HOSPITAL");
}

// -----------------------------------------------------------------------------
// Test Suite 4: Resource Ownership & Access Control (Tests 15, 16, 17)
// -----------------------------------------------------------------------------
void testResourceOwnershipAndAccess(
    std::shared_ptr<SqliteEmergencyRepository> emerRepo,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo,
    std::shared_ptr<EmergencyDispatcher> dispatcher,
    std::shared_ptr<AuthService> authService) {
    (void)ambRepo;
    (void)hospRepo;

    std::cout << "[Test Suite 4/5] Testing Resource Ownership & Access Isolation...\n";

    // Register two distinct patients
    auto p1 = authService->registerPatient("Patient One", "+91 98000 11111", "patient_one", "PassOne123!");
    auto p2 = authService->registerPatient("Patient Two", "+91 98000 22222", "patient_two", "PassTwo123!");

    // Patient 1 dispatches an emergency
    auto disp1 = dispatcher->dispatchEmergency(
        "Patient One", "+91 98000 11111", "Clock Tower", 30.3244, 78.0418, "Fever and weakness", "MEDIUM", p1.user.userId);
    TEST_ASSERT(disp1.success, "Patient 1 emergency dispatch succeeded");
    std::string em1Id = disp1.emergency.emergencyId;

    // Patient 2 dispatches an emergency
    auto disp2 = dispatcher->dispatchEmergency(
        "Patient Two", "+91 98000 22222", "ISBT", 30.2884, 78.0069, "Chest pain", "CRITICAL", p2.user.userId);
    TEST_ASSERT(disp2.success, "Patient 2 emergency dispatch succeeded");
    std::string em2Id = disp2.emergency.emergencyId;

    // Verify Patient 1 only sees their own emergencies
    auto p1List = emerRepo->findByUserId(p1.user.userId);
    TEST_ASSERT(p1List.size() == 1, "Patient 1 sees exactly 1 emergency");
    TEST_ASSERT(p1List[0].emergencyId == em1Id, "Patient 1 sees their own emergency EM1");

    auto p2List = emerRepo->findByUserId(p2.user.userId);
    TEST_ASSERT(p2List.size() == 1, "Patient 2 sees exactly 1 emergency");
    TEST_ASSERT(p2List[0].emergencyId == em2Id, "Patient 2 sees their own emergency EM2");

    // Verify Hospital isolation
    std::string hosp1Id = disp1.assignedHospital.hospitalId;
    std::string hosp2Id = disp2.assignedHospital.hospitalId;

    auto h1List = emerRepo->findByHospitalId(hosp1Id);
    bool foundEm1 = false, foundEm2 = false;
    for (const auto& e : h1List) {
        if (e.emergencyId == em1Id) foundEm1 = true;
        if (e.emergencyId == em2Id) foundEm2 = true;
    }
    TEST_ASSERT(foundEm1, "Hospital 1 sees EM1 routed to it");
    if (hosp1Id != hosp2Id) {
        TEST_ASSERT(!foundEm2, "Hospital 1 does NOT see EM2 routed to another hospital");
    }

    // Verify Driver isolation
    std::string amb1Id = disp1.assignedAmbulance.ambulanceId;
    auto amb1List = emerRepo->findByAmbulanceId(amb1Id);
    bool amb1HasEm1 = false, amb1HasEm2 = false;
    for (const auto& e : amb1List) {
        if (e.emergencyId == em1Id) amb1HasEm1 = true;
        if (e.emergencyId == em2Id) amb1HasEm2 = true;
    }
    TEST_ASSERT(amb1HasEm1, "Ambulance 1 sees EM1 assigned to it");
    TEST_ASSERT(!amb1HasEm2, "Ambulance 1 does NOT see EM2 assigned to a different ambulance");
}

// -----------------------------------------------------------------------------
// Test Suite 5: Password Reset & Session Invalidation
// -----------------------------------------------------------------------------
void testPasswordResetAndSessionInvalidation(std::shared_ptr<AuthService> authService) {
    std::cout << "[Test Suite 5/5] Testing Password Reset & Session Invalidation...\n";

    AuthUser adminUser;
    adminUser.role = "ADMIN";
    adminUser.username = "admin";

    // Register user
    auto reg = authService->registerPatient("Reset User", "+91 97777 00000", "reset_user", "OldPassword123!");
    TEST_ASSERT(reg.success, "Registration for reset user succeeded");

    // Login to get a token
    auto login = authService->login("reset_user", "OldPassword123!");
    TEST_ASSERT(login.success, "Login with old password succeeded");
    std::string oldToken = login.token;

    // Verify token is valid
    TEST_ASSERT(authService->validateToken(oldToken).has_value(), "Old session token is valid");

    // Admin resets user password
    bool resetOk = authService->resetPassword(adminUser, reg.user.userId, "NewBrandPassword2026!");
    TEST_ASSERT(resetOk, "Admin password reset succeeded");

    // Old token must now be invalid!
    TEST_ASSERT(!authService->validateToken(oldToken).has_value(), "Existing session token is revoked upon password reset");

    // Old password must fail
    auto oldLogin = authService->login("reset_user", "OldPassword123!");
    TEST_ASSERT(!oldLogin.success, "Old password rejected after reset");

    // New password succeeds
    auto newLogin = authService->login("reset_user", "NewBrandPassword2026!");
    TEST_ASSERT(newLogin.success, "New password accepted after reset");
}

// -----------------------------------------------------------------------------
// Main Test Runner
// -----------------------------------------------------------------------------
int main() {
    std::cout << "========================================================================\n";
    std::cout << "  JEEVAN MARG -- PHASE 3 AUTHENTICATION & SECURITY TEST RUNNER\n";
    std::cout << "  Testing PBKDF2 Password Hashing, Sessions, RBAC & Resource Ownership\n";
    std::cout << "========================================================================\n\n";

    std::string seedDir = "seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "m3_navigation/seed";
    if (!fs::exists(seedDir + "/locations.csv")) seedDir = "../seed";

    std::string schemaFile = locateSchema();

    // 1. Setup in-memory database
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

    // 3. Graph & Pathfinding & Dispatcher & AuthService
    auto edges = roadRepo->findAll();
    auto graph = RoadGraphBuilder::buildGraph(*locRepo, edges, true);
    auto pathFinder = std::make_shared<ShortestPathFinder>(graph);
    auto estimator = std::make_shared<TransportEstimator>();
    auto dispatcher = std::make_shared<EmergencyDispatcher>(
        db, emerRepo, ambRepo, hospRepo, graph, pathFinder, estimator);

    auto authService = std::make_shared<AuthService>(userRepo, sessionRepo, ambRepo, hospRepo);

    // 4. Run test suites
    testCryptoAndTokens(userRepo, sessionRepo, authService);
    testPatientRegistrationAndLogin(userRepo, authService);
    testAdminProvisioning(authService);
    testResourceOwnershipAndAccess(emerRepo, ambRepo, hospRepo, dispatcher, authService);
    testPasswordResetAndSessionInvalidation(authService);

    std::cout << "\n========================================================================\n";
    std::cout << "  PHASE 3 AUTH TEST SUMMARY: " << g_testsPassed << " / " << g_testsRun << " ASSERTIONS PASSED (";
    if (g_testsPassed == g_testsRun) {
        std::cout << "100% SUCCESS - ALL PHASE 3 SECURITY TESTS GREEN!)\n";
    } else {
        std::cout << (g_testsRun - g_testsPassed) << " FAILED)\n";
    }
    std::cout << "========================================================================\n\n";

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
