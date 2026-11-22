#pragma once

#include "Database.h"
#include "../structure/RoadEdge.h"
#include <vector>

class SqliteRoadRepository {
public:
    explicit SqliteRoadRepository(Database& db);
    std::vector<RoadEdge> findAll() const;
    bool save(const RoadEdge& edge);
    size_t count() const;
private:
    Database& db_;
};

