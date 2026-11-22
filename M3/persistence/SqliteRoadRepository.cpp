#include "SqliteRoadRepository.h"

SqliteRoadRepository::SqliteRoadRepository(Database& db)
    : db_(db) {}

std::vector<RoadEdge> SqliteRoadRepository::findAll() const {
    std::vector<RoadEdge> edges;
    auto stmt = db_.prepare(R"(
        SELECT from_node_id, to_node_id, distance_km, road_name, road_type
        FROM roads
        ORDER BY road_id
    )");
    while (stmt.step() == SQLITE_ROW) {
        std::string from = stmt.colText(0);
        std::string to = stmt.colText(1);
        double dist = stmt.colDouble(2);
        std::string name = stmt.colText(3);
        std::string typeStr = stmt.colText(4);
        RoadType rtype = stringToRoadType(typeStr);
        edges.emplace_back(from, to, dist, name, rtype);
    }
    return edges;
}

bool SqliteRoadRepository::save(const RoadEdge& edge) {
    auto stmt = db_.prepare(R"(
        INSERT OR IGNORE INTO roads(from_node_id, to_node_id, distance_km, road_name, road_type)
        VALUES (?, ?, ?, ?, ?)
    )");
    stmt.bind(1, edge.getFromNodeId());
    stmt.bind(2, edge.getToNodeId());
    stmt.bind(3, edge.getDistanceKm());
    stmt.bind(4, edge.getRoadName());
    stmt.bind(5, roadTypeToString(edge.getRoadType()));
    return stmt.step() == SQLITE_DONE;
}

size_t SqliteRoadRepository::count() const {
    auto stmt = db_.prepare("SELECT COUNT(*) FROM roads");
    if (stmt.step() == SQLITE_ROW) {
        return static_cast<size_t>(stmt.colInt64(0));
    }
    return 0;
}

