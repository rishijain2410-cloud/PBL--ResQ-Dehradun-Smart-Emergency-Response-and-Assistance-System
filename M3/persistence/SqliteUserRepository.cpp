#include "SqliteUserRepository.h"
#include <stdexcept>

SqliteUserRepository::SqliteUserRepository(Database& db)
    : db_(db) {}

UserRecord SqliteUserRepository::rowToRecord(DbStatement& stmt) const {
    UserRecord u;
    u.userId            = stmt.colInt64(0);
    u.role              = stmt.colText(1);
    u.username          = stmt.colText(2);
    u.passwordHash      = stmt.colText(3);
    u.displayName       = stmt.colText(4);
    u.phone             = stmt.colText(5);
    u.linkedAmbulanceId = stmt.colText(6);
    u.linkedHospitalId  = stmt.colText(7);
    u.isActive          = stmt.colBool(8);
    u.createdAt         = stmt.colText(9);
    return u;
}

UserRecord SqliteUserRepository::create(const UserRecord& user) {
    auto stmt = db_.prepare(R"(
        INSERT INTO users(role, username, password_hash, display_name, phone,
                          linked_ambulance_id, linked_hospital_id, is_active)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");

    stmt.bind(1, user.role);
    stmt.bind(2, user.username);
    stmt.bind(3, user.passwordHash);
    stmt.bind(4, user.displayName);
    stmt.bind(5, user.phone);

    if (user.linkedAmbulanceId.empty()) {
        stmt.bindNull(6);
    } else {
        stmt.bind(6, user.linkedAmbulanceId);
    }

    if (user.linkedHospitalId.empty()) {
        stmt.bindNull(7);
    } else {
        stmt.bind(7, user.linkedHospitalId);
    }

    stmt.bind(8, user.isActive ? 1 : 0);

    stmt.step();

    UserRecord out = user;
    out.userId = db_.lastInsertRowId();
    return out;
}

std::optional<UserRecord> SqliteUserRepository::findById(int64_t userId) const {
    auto stmt = db_.prepare(R"(
        SELECT user_id, role, username, password_hash, display_name, phone,
               COALESCE(linked_ambulance_id, ''),
               COALESCE(linked_hospital_id, ''),
               is_active, created_at
        FROM users
        WHERE user_id = ?
    )");
    stmt.bind(1, userId);
    if (stmt.step() == SQLITE_ROW) {
        return rowToRecord(stmt);
    }
    return std::nullopt;
}

std::optional<UserRecord> SqliteUserRepository::findByUsername(const std::string& username) const {
    auto stmt = db_.prepare(R"(
        SELECT user_id, role, username, password_hash, display_name, phone,
               COALESCE(linked_ambulance_id, ''),
               COALESCE(linked_hospital_id, ''),
               is_active, created_at
        FROM users
        WHERE username = ?
    )");
    stmt.bind(1, username);
    if (stmt.step() == SQLITE_ROW) {
        return rowToRecord(stmt);
    }
    return std::nullopt;
}

std::vector<UserRecord> SqliteUserRepository::findAll() const {
    std::vector<UserRecord> out;
    auto stmt = db_.prepare(R"(
        SELECT user_id, role, username, password_hash, display_name, phone,
               COALESCE(linked_ambulance_id, ''),
               COALESCE(linked_hospital_id, ''),
               is_active, created_at
        FROM users
        ORDER BY user_id ASC
    )");
    while (stmt.step() == SQLITE_ROW) {
        out.push_back(rowToRecord(stmt));
    }
    return out;
}

std::vector<UserRecord> SqliteUserRepository::findByRole(const std::string& role) const {
    std::vector<UserRecord> out;
    auto stmt = db_.prepare(R"(
        SELECT user_id, role, username, password_hash, display_name, phone,
               COALESCE(linked_ambulance_id, ''),
               COALESCE(linked_hospital_id, ''),
               is_active, created_at
        FROM users
        WHERE role = ?
        ORDER BY user_id ASC
    )");
    stmt.bind(1, role);
    while (stmt.step() == SQLITE_ROW) {
        out.push_back(rowToRecord(stmt));
    }
    return out;
}

std::optional<UserRecord> SqliteUserRepository::findByAmbulanceId(const std::string& ambulanceId) const {
    auto stmt = db_.prepare(R"(
        SELECT user_id, role, username, password_hash, display_name, phone,
               COALESCE(linked_ambulance_id, ''),
               COALESCE(linked_hospital_id, ''),
               is_active, created_at
        FROM users
        WHERE linked_ambulance_id = ? AND is_active = 1
    )");
    stmt.bind(1, ambulanceId);
    if (stmt.step() == SQLITE_ROW) {
        return rowToRecord(stmt);
    }
    return std::nullopt;
}

std::optional<UserRecord> SqliteUserRepository::findByHospitalId(const std::string& hospitalId) const {
    auto stmt = db_.prepare(R"(
        SELECT user_id, role, username, password_hash, display_name, phone,
               COALESCE(linked_ambulance_id, ''),
               COALESCE(linked_hospital_id, ''),
               is_active, created_at
        FROM users
        WHERE linked_hospital_id = ? AND is_active = 1
    )");
    stmt.bind(1, hospitalId);
    if (stmt.step() == SQLITE_ROW) {
        return rowToRecord(stmt);
    }
    return std::nullopt;
}

bool SqliteUserRepository::updateStatus(int64_t userId, bool isActive) {
    auto stmt = db_.prepare("UPDATE users SET is_active = ? WHERE user_id = ?");
    stmt.bind(1, isActive ? 1 : 0);
    stmt.bind(2, userId);
    return stmt.step() == SQLITE_DONE;
}

bool SqliteUserRepository::updatePassword(int64_t userId, const std::string& newPasswordHash) {
    auto stmt = db_.prepare("UPDATE users SET password_hash = ? WHERE user_id = ?");
    stmt.bind(1, newPasswordHash);
    stmt.bind(2, userId);
    return stmt.step() == SQLITE_DONE;
}

size_t SqliteUserRepository::count() const {
    auto stmt = db_.prepare("SELECT COUNT(*) FROM users");
    if (stmt.step() == SQLITE_ROW) {
        return static_cast<size_t>(stmt.colInt64(0));
    }
    return 0;
}
