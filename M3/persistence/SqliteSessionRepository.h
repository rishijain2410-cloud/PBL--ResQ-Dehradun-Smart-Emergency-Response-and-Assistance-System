#ifndef M3_PERSISTENCE_SQLITE_SESSION_REPOSITORY_H
#define M3_PERSISTENCE_SQLITE_SESSION_REPOSITORY_H

#include "Database.h"
#include "PersistenceRecords.h"
#include <string>
#include <optional>

class SqliteSessionRepository {
public:
    explicit SqliteSessionRepository(Database& db);

    // Creates a new session storing the token hash and an expiration timestamp (default 24h)
    SessionRecord createSession(int64_t userId, const std::string& tokenHash, int hoursValid = 24);

    // Validates token hash against non-revoked, unexpired sessions and active users
    std::optional<AuthUser> findValidSession(const std::string& tokenHash) const;

    // Revokes a single session (e.g. on logout)
    bool revokeSession(const std::string& tokenHash);

    // Revokes all sessions for a user (e.g. on password reset or deactivation)
    bool revokeAllUserSessions(int64_t userId);

    // Cleanup expired sessions older than a certain duration
    size_t cleanupExpired();

private:
    Database& db_;
};

#endif // M3_PERSISTENCE_SQLITE_SESSION_REPOSITORY_H
