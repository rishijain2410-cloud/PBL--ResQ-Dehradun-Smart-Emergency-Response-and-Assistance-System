#include "SqliteLocationRepository.h"

SqliteLocationRepository::SqliteLocationRepository(Database& db)
    : db_(db) {}

Location SqliteLocationRepository::rowToLocation(DbStatement& stmt) const {
    std::string id = stmt.colText(0);
    std::string name = stmt.colText(1);
    double lat = stmt.colDouble(2);
    double lng = stmt.colDouble(3);
    std::string typeStr = stmt.colText(4);
    std::string addr = stmt.colText(5);
    LocationType type = stringToLocationType(typeStr);
    return Location(id, name, lat, lng, type, addr);
}

bool SqliteLocationRepository::save(const Location& location) {
    auto stmt = db_.prepare(R"(
        INSERT OR REPLACE INTO locations(location_id, name, latitude, longitude, type, address)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    stmt.bind(1, location.getId());
    stmt.bind(2, location.getName());
    stmt.bind(3, location.getLatitude());
    stmt.bind(4, location.getLongitude());
    stmt.bind(5, locationTypeToString(location.getType()));
    stmt.bind(6, location.getAddress());
    return stmt.step() == SQLITE_DONE;
}

std::optional<Location> SqliteLocationRepository::findById(const std::string& id) const {
    auto stmt = db_.prepare(R"(
        SELECT location_id, name, latitude, longitude, type, address
        FROM locations
        WHERE location_id = ?
    )");
    stmt.bind(1, id);
    if (stmt.step() == SQLITE_ROW) {
        return rowToLocation(stmt);
    }
    return std::nullopt;
}

std::vector<Location> SqliteLocationRepository::findAll() const {
    std::vector<Location> results;
    auto stmt = db_.prepare(R"(
        SELECT location_id, name, latitude, longitude, type, address
        FROM locations
        ORDER BY location_id
    )");
    while (stmt.step() == SQLITE_ROW) {
        results.push_back(rowToLocation(stmt));
    }
    return results;
}

std::vector<Location> SqliteLocationRepository::findByType(LocationType type) const {
    std::vector<Location> results;
    auto stmt = db_.prepare(R"(
        SELECT location_id, name, latitude, longitude, type, address
        FROM locations
        WHERE type = ?
        ORDER BY location_id
    )");
    stmt.bind(1, locationTypeToString(type));
    while (stmt.step() == SQLITE_ROW) {
        results.push_back(rowToLocation(stmt));
    }
    return results;
}

size_t SqliteLocationRepository::count() const {
    auto stmt = db_.prepare("SELECT COUNT(*) FROM locations");
    if (stmt.step() == SQLITE_ROW) {
        return static_cast<size_t>(stmt.colInt64(0));
    }
    return 0;
}

void SqliteLocationRepository::clear() {
    db_.exec("DELETE FROM locations");
}

