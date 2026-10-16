#pragma once

#include "model/Location.h"

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>

class ILocationRepository {
public:
    virtual ~ILocationRepository() = default;

    virtual std::optional<Location> findById(const std::string& id) const = 0;

    virtual std::vector<Location> findAll() const = 0;

    virtual std::vector<Location> findByType(LocationType type) const = 0;

    virtual bool save(const Location& location) = 0;
};

class InMemoryLocationRepository : public ILocationRepository {
private:
    std::unordered_map<std::string, Location> itemMapById;
    std::vector<Location> itemList;
    std::unordered_set<std::string> uniqueIdSet;

public:
    InMemoryLocationRepository() = default;
    ~InMemoryLocationRepository() override = default;

    std::optional<Location> findById(const std::string& id) const override;
    std::vector<Location> findAll() const override;
    std::vector<Location> findByType(LocationType type) const override;
    bool save(const Location& location) override;

    size_t size() const;
    void clear();
};
