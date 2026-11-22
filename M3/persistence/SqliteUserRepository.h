#ifndef M3_PERSISTENCE_SQLITE_USER_REPOSITORY_H
#define M3_PERSISTENCE_SQLITE_USER_REPOSITORY_H

#include "Database.h"
#include "PersistenceRecords.h"
#include <string>
#include <vector>
#include <optional>

class SqliteUserRepository {
public:
    explicit SqliteUserRepository(Database& db);

    // Creates a new user record. Throws if username already exists.
    UserRecord create(const UserRecord& user);

    // Lookup
    std::optional<UserRecord> findById(int64_t userId) const;
    std::optional<UserRecord> findByUsername(const std::string& username) const;
    std::vector<UserRecord> findAll() const;
    std::vector<UserRecord> findByRole(const std::string& role) const;
    std::optional<UserRecord> findByAmbulanceId(const std::string& ambulanceId) const;
    std::optional<UserRecord> findByHospitalId(const std::string& hospitalId) const;

    // Updates
    bool updateStatus(int64_t userId, bool isActive);
    bool updatePassword(int64_t userId, const std::string& newPasswordHash);

    // Count
    size_t count() const;

private:
    Database& db_;
    UserRecord rowToRecord(DbStatement& stmt) const;
};

#endif // M3_PERSISTENCE_SQLITE_USER_REPOSITORY_H
