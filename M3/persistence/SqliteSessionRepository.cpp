#include "SqliteSessionRepository.h"

SqliteSessionRepository::SqliteSessionRepository(Database& db)
    : db_(db) {}

SessionRecord SqliteSessionRepository::createSession(int64_t userId, const std::string& tokenHash, int hoursValid) {
    auto stmt = db_.prepare(R"(
        INSERT INTO sessions(user_id, token_hash, created_at, expires_at, is_revoked)
        VALUES (?, ?, datetime('now'), datetime('now', '+' || ? || ' hours'), 0)
    )");

    stmt.bind(1, userId);
    stmt.bind(2, tokenHash);
    stmt.bind(3, std::to_string(hoursValid));
    stmt.step();

    int64_t sessId = db_.lastInsertRowId();

    SessionRecord r;
    r.sessionId = sessId;
    r.userId = userId;
    r.tokenHash = tokenHash;
    r.isRevoked = false;

    // Fetch timestamps
    auto fetchStmt = db_.prepare("SELECT created_at, expires_at FROM sessions WHERE session_id = ?");
    fetchStmt.bind(1, sessId);
    if (fetchStmt.step() == SQLITE_ROW) {
        r.createdAt = fetchStmt.colText(0);
        r.expiresAt = fetchStmt.colText(1);
    }
    return r;
}

std::optional<AuthUser> SqliteSessionRepository::findValidSession(const std::string& tokenHash) const {
    auto stmt = db_.prepare(R"(
        SELECT u.user_id, u.role, u.username, u.display_name, u.phone,
               COALESCE(u.linked_ambulance_id, ''),
               COALESCE(u.linked_hospital_id, ''),
               u.is_active
        FROM sessions s
        JOIN users u ON s.user_id = u.user_id
        WHERE s.token_hash = ?
          AND s.is_revoked = 0
          AND s.expires_at > datetime('now')
          AND u.is_active = 1
    )");

    stmt.bind(1, tokenHash);
    if (stmt.step() == SQLITE_ROW) {
        AuthUser u;
        u.userId            = stmt.colInt64(0);
        u.role              = stmt.colText(1);
        u.username          = stmt.colText(2);
        u.displayName       = stmt.colText(3);
        u.phone             = stmt.colText(4);
        u.linkedAmbulanceId = stmt.colText(5);
        u.linkedHospitalId  = stmt.colText(6);
        u.isActive          = stmt.colBool(7);
        return u;
    }
    return std::nullopt;
}

bool SqliteSessionRepository::revokeSession(const std::string& tokenHash) {
    auto stmt = db_.prepare("UPDATE sessions SET is_revoked = 1 WHERE token_hash = ?");
    stmt.bind(1, tokenHash);
    return stmt.step() == SQLITE_DONE;
}

bool SqliteSessionRepository::revokeAllUserSessions(int64_t userId) {
    auto stmt = db_.prepare("UPDATE sessions SET is_revoked = 1 WHERE user_id = ?");
    stmt.bind(1, userId);
    return stmt.step() == SQLITE_DONE;
}

size_t SqliteSessionRepository::cleanupExpired() {
    auto stmt = db_.prepare("DELETE FROM sessions WHERE expires_at <= datetime('now') OR is_revoked = 1");
    stmt.step();
    return 0;
}
