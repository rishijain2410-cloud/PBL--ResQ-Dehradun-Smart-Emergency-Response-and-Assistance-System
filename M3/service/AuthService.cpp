#include "AuthService.h"
#include <algorithm>
#include <cctype>
#include <regex>

namespace {
bool isValidUsername(const std::string& u) {
    if (u.size() < 3 || u.size() > 50) return false;
    for (char c : u) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-' && c != '.') {
            return false;
        }
    }
    return true;
}

bool isValidPassword(const std::string& p) {
    return p.size() >= 8;
}
} // anonymous namespace

AuthService::AuthService(
    std::shared_ptr<SqliteUserRepository> userRepo,
    std::shared_ptr<SqliteSessionRepository> sessionRepo,
    std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
    std::shared_ptr<SqliteHospitalRepository> hospRepo)
    : userRepo_(userRepo), sessionRepo_(sessionRepo),
      ambulanceRepo_(ambRepo), hospitalRepo_(hospRepo) {}

AuthResult AuthService::registerPatient(
    const std::string& displayName,
    const std::string& phone,
    const std::string& username,
    const std::string& password) {

    AuthResult res;

    if (displayName.empty()) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Full name cannot be empty.";
        return res;
    }
    if (phone.empty() || phone.size() < 7) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "A valid phone number is required.";
        return res;
    }
    if (!isValidUsername(username)) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Username must be 3-50 characters long and contain only letters, numbers, '.', '_', or '-'.";
        return res;
    }
    if (!isValidPassword(password)) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Password must be at least 8 characters long.";
        return res;
    }

    // Check duplicate username
    if (userRepo_->findByUsername(username).has_value()) {
        res.success = false;
        res.errorCode = "DUPLICATE_USERNAME";
        res.message = "Username '" + username + "' is already registered.";
        return res;
    }

    try {
        UserRecord u;
        u.role = "PATIENT"; // Strictly force role to PATIENT
        u.username = username;
        u.displayName = displayName;
        u.phone = phone;
        u.passwordHash = CryptoUtil::hashPassword(password);
        u.isActive = true;

        UserRecord created = userRepo_->create(u);

        // Generate session token
        std::string rawToken = CryptoUtil::generateToken(32);
        std::string tokenHash = CryptoUtil::sha256Hex(rawToken);

        sessionRepo_->createSession(created.userId, tokenHash, 24);

        res.success = true;
        res.message = "Patient account registered successfully.";
        res.token = rawToken;
        res.user.userId = created.userId;
        res.user.role = created.role;
        res.user.username = created.username;
        res.user.displayName = created.displayName;
        res.user.phone = created.phone;
        res.user.isActive = created.isActive;

    } catch (const std::exception& e) {
        res.success = false;
        res.errorCode = "DATABASE_ERROR";
        res.message = std::string("Registration failed: ") + e.what();
    }

    return res;
}

AuthResult AuthService::login(
    const std::string& username,
    const std::string& password) {

    AuthResult res;

    if (username.empty() || password.empty()) {
        res.success = false;
        res.errorCode = "INVALID_CREDENTIALS";
        res.message = "Username and password are required.";
        return res;
    }

    auto userOpt = userRepo_->findByUsername(username);
    if (!userOpt.has_value()) {
        res.success = false;
        res.errorCode = "INVALID_CREDENTIALS";
        res.message = "Invalid username or password.";
        return res;
    }

    const auto& user = *userOpt;

    if (!user.isActive) {
        res.success = false;
        res.errorCode = "ACCOUNT_DISABLED";
        res.message = "This account has been deactivated. Please contact an administrator.";
        return res;
    }

    if (!CryptoUtil::verifyPassword(password, user.passwordHash)) {
        res.success = false;
        res.errorCode = "INVALID_CREDENTIALS";
        res.message = "Invalid username or password.";
        return res;
    }

    try {
        std::string rawToken = CryptoUtil::generateToken(32);
        std::string tokenHash = CryptoUtil::sha256Hex(rawToken);

        sessionRepo_->createSession(user.userId, tokenHash, 24);

        res.success = true;
        res.message = "Login successful.";
        res.token = rawToken;
        res.user.userId = user.userId;
        res.user.role = user.role;
        res.user.username = user.username;
        res.user.displayName = user.displayName;
        res.user.phone = user.phone;
        res.user.linkedAmbulanceId = user.linkedAmbulanceId;
        res.user.linkedHospitalId = user.linkedHospitalId;
        res.user.isActive = user.isActive;

    } catch (const std::exception& e) {
        res.success = false;
        res.errorCode = "SERVER_ERROR";
        res.message = std::string("Session creation failed: ") + e.what();
    }

    return res;
}

bool AuthService::logout(const std::string& rawToken) {
    if (rawToken.empty()) return false;
    std::string tokenHash = CryptoUtil::sha256Hex(rawToken);
    return sessionRepo_->revokeSession(tokenHash);
}

std::optional<AuthUser> AuthService::validateToken(const std::string& rawToken) const {
    if (rawToken.empty()) return std::nullopt;
    std::string tokenHash = CryptoUtil::sha256Hex(rawToken);
    return sessionRepo_->findValidSession(tokenHash);
}

AuthResult AuthService::createDriver(
    const AuthUser& adminUser,
    const std::string& username,
    const std::string& password,
    const std::string& displayName,
    const std::string& phone,
    const std::string& ambulanceId) {

    AuthResult res;

    if (adminUser.role != "ADMIN") {
        res.success = false;
        res.errorCode = "FORBIDDEN";
        res.message = "Only administrators can create driver accounts.";
        return res;
    }
    if (!isValidUsername(username)) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Invalid username format.";
        return res;
    }
    if (!isValidPassword(password)) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Password must be at least 8 characters long.";
        return res;
    }
    if (ambulanceId.empty()) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Ambulance ID is required for driver account.";
        return res;
    }

    // Verify ambulance exists
    if (ambulanceRepo_) {
        auto ambOpt = ambulanceRepo_->findById(ambulanceId);
        if (!ambOpt.has_value()) {
            res.success = false;
            res.errorCode = "RESOURCE_NOT_FOUND";
            res.message = "Ambulance ID '" + ambulanceId + "' does not exist.";
            return res;
        }
    }

    // Check duplicate driver-to-ambulance assignment
    auto existingDriver = userRepo_->findByAmbulanceId(ambulanceId);
    if (existingDriver.has_value()) {
        res.success = false;
        res.errorCode = "AMBULANCE_ALREADY_ASSIGNED";
        res.message = "Ambulance '" + ambulanceId + "' is already assigned to active driver '" + existingDriver->username + "'.";
        return res;
    }

    // Check duplicate username
    if (userRepo_->findByUsername(username).has_value()) {
        res.success = false;
        res.errorCode = "DUPLICATE_USERNAME";
        res.message = "Username '" + username + "' is already registered.";
        return res;
    }

    try {
        UserRecord u;
        u.role = "DRIVER";
        u.username = username;
        u.displayName = displayName.empty() ? username : displayName;
        u.phone = phone;
        u.linkedAmbulanceId = ambulanceId;
        u.passwordHash = CryptoUtil::hashPassword(password);
        u.isActive = true;

        UserRecord created = userRepo_->create(u);

        res.success = true;
        res.message = "Driver account created successfully.";
        res.user.userId = created.userId;
        res.user.role = created.role;
        res.user.username = created.username;
        res.user.displayName = created.displayName;
        res.user.phone = created.phone;
        res.user.linkedAmbulanceId = created.linkedAmbulanceId;
        res.user.isActive = created.isActive;

    } catch (const std::exception& e) {
        res.success = false;
        res.errorCode = "DATABASE_ERROR";
        res.message = std::string("Driver creation failed: ") + e.what();
    }

    return res;
}

AuthResult AuthService::createHospital(
    const AuthUser& adminUser,
    const std::string& username,
    const std::string& password,
    const std::string& displayName,
    const std::string& phone,
    const std::string& hospitalId) {

    AuthResult res;

    if (adminUser.role != "ADMIN") {
        res.success = false;
        res.errorCode = "FORBIDDEN";
        res.message = "Only administrators can create hospital accounts.";
        return res;
    }
    if (!isValidUsername(username)) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Invalid username format.";
        return res;
    }
    if (!isValidPassword(password)) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Password must be at least 8 characters long.";
        return res;
    }
    if (hospitalId.empty()) {
        res.success = false;
        res.errorCode = "VALIDATION_ERROR";
        res.message = "Hospital ID is required.";
        return res;
    }

    // Verify hospital exists
    if (hospitalRepo_) {
        auto hospOpt = hospitalRepo_->findById(hospitalId);
        if (!hospOpt.has_value()) {
            res.success = false;
            res.errorCode = "RESOURCE_NOT_FOUND";
            res.message = "Hospital ID '" + hospitalId + "' does not exist.";
            return res;
        }
    }

    // Check duplicate username
    if (userRepo_->findByUsername(username).has_value()) {
        res.success = false;
        res.errorCode = "DUPLICATE_USERNAME";
        res.message = "Username '" + username + "' is already registered.";
        return res;
    }

    try {
        UserRecord u;
        u.role = "HOSPITAL";
        u.username = username;
        u.displayName = displayName.empty() ? username : displayName;
        u.phone = phone;
        u.linkedHospitalId = hospitalId;
        u.passwordHash = CryptoUtil::hashPassword(password);
        u.isActive = true;

        UserRecord created = userRepo_->create(u);

        res.success = true;
        res.message = "Hospital account created successfully.";
        res.user.userId = created.userId;
        res.user.role = created.role;
        res.user.username = created.username;
        res.user.displayName = created.displayName;
        res.user.phone = created.phone;
        res.user.linkedHospitalId = created.linkedHospitalId;
        res.user.isActive = created.isActive;

    } catch (const std::exception& e) {
        res.success = false;
        res.errorCode = "DATABASE_ERROR";
        res.message = std::string("Hospital account creation failed: ") + e.what();
    }

    return res;
}

bool AuthService::setUserActive(
    const AuthUser& adminUser,
    int64_t targetUserId,
    bool isActive) {

    if (adminUser.role != "ADMIN") return false;

    bool ok = userRepo_->updateStatus(targetUserId, isActive);
    if (ok && !isActive) {
        // Revoke active sessions if user is disabled
        sessionRepo_->revokeAllUserSessions(targetUserId);
    }
    return ok;
}

bool AuthService::resetPassword(
    const AuthUser& adminUser,
    int64_t targetUserId,
    const std::string& newPassword) {

    if (adminUser.role != "ADMIN") return false;
    if (!isValidPassword(newPassword)) return false;

    std::string newHash = CryptoUtil::hashPassword(newPassword);
    bool ok = userRepo_->updatePassword(targetUserId, newHash);
    if (ok) {
        // Invalidate all existing sessions on password reset
        sessionRepo_->revokeAllUserSessions(targetUserId);
    }
    return ok;
}
