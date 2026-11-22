#pragma once
// SqliteLocationRepository.h — SQLite-backed ILocationRepository

#include "../repository/ILocationRepository.h"
#include "Database.h"

class SqliteLocationRepository : public ILocationRepository {
public:
    explicit SqliteLocationRepository(Database& db);

    bool save(const Location& location) override;
    std::optional<Location> findById(const std::string& id) const override;
    std::vector<Location> findAll() const override;
    std::vector<Location> findByType(LocationType type) const override;
    size_t count() const override;
    void clear() override;

private:
    Database& db_;
    // Reads cols 0..5: id, name, lat, lng, type_str, address
    Location rowToLocation(DbStatement& stmt) const;
};
