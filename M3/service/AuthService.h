#ifndef M3_SERVICE_AUTH_SERVICE_H
#define M3_SERVICE_AUTH_SERVICE_H

#include "../persistence/Database.h"
#include "../persistence/SqliteUserRepository.h"
#include "../persistence/SqliteSessionRepository.h"
#include "../persistence/SqliteAmbulanceRepository.h"
#include "../persistence/SqliteHospitalRepository.h"
#include "../util/CryptoUtil.h"

#include <memory>
#include <string>
#include <optional>
#include <vector>

struct AuthResult {
    bool success = false;
    std::string errorCode;
    std::string message;
    std::string token;      // Raw session token (returned only on login/register)
    AuthUser user;
};

class AuthService {
public:
    AuthService(
        std::shared_ptr<SqliteUserRepository> userRepo,
        std::shared_ptr<SqliteSessionRepository> sessionRepo,
        std::shared_ptr<SqliteAmbulanceRepository> ambRepo,
        std::shared_ptr<SqliteHospitalRepository> hospRepo);

    // Public Registration (strictly forced to PATIENT role)
    AuthResult registerPatient(
        const std::string& displayName,
        const std::string& phone,
        const std::string& username,
        const std::string& password);

    // User Login (for any role: PATIENT, DRIVER, HOSPITAL, ADMIN)
    AuthResult login(
        const std::string& username,
        const std::string& password);

    // Logout: revokes the current session token
    bool logout(const std::string& rawToken);

    // Token Validation: resolves AuthUser from raw Bearer token (constant time check + DB lookup)
    std::optional<AuthUser> validateToken(const std::string& rawToken) const;

    // Admin Account Creation: DRIVER
    AuthResult createDriver(
        const AuthUser& adminUser,
        const std::string& username,
        const std::string& password,
        const std::string& displayName,
        const std::string& phone,
        const std::string& ambulanceId);

    // Admin Account Creation: HOSPITAL
    AuthResult createHospital(
        const AuthUser& adminUser,
        const std::string& username,
        const std::string& password,
        const std::string& displayName,
        const std::string& phone,
        const std::string& hospitalId);

    // Admin Account Management: Activate/Deactivate user
    bool setUserActive(
        const AuthUser& adminUser,
        int64_t targetUserId,
        bool isActive);

    // Admin Account Management: Reset user password
    bool resetPassword(
        const AuthUser& adminUser,
        int64_t targetUserId,
        const std::string& newPassword);

private:
    std::shared_ptr<SqliteUserRepository> userRepo_;
    std::shared_ptr<SqliteSessionRepository> sessionRepo_;
    std::shared_ptr<SqliteAmbulanceRepository> ambulanceRepo_;
    std::shared_ptr<SqliteHospitalRepository> hospitalRepo_;
};

#endif // M3_SERVICE_AUTH_SERVICE_H
