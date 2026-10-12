#pragma once

#include "model/Location.h"

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>

/**
 * ============================================================================
 * OOP Concept: Interface / Abstraction & Polymorphism
 * - ILocationRepository defines an abstract contract (pure virtual interface)
 *   that decouples domain services and controllers from data storage mechanisms.
 * - Conforms to the Dependency Inversion Principle: Services depend strictly on
 *   this interface, allowing InMemoryLocationRepository to be swapped for a
 *   database adapter (e.g. SQLite/PostgreSQL) during final integration without
 *   touching business logic.
 * ============================================================================
 */
class ILocationRepository {
public:
    virtual ~ILocationRepository() = default;

    /**
     * Finds a location by unique ID.
     * @param id Canonical location ID (e.g., "LOC-001")
     * @return std::optional containing the Location if found, or std::nullopt.
     */
    virtual std::optional<Location> findById(const std::string& id) const = 0;

    /**
     * Retrieves all locations in insertion order.
     * @return std::vector<Location> containing all locations.
     */
    virtual std::vector<Location> findAll() const = 0;

    /**
     * Filters locations by LocationType.
     * @param type The LocationType enum to filter by.
     * @return std::vector<Location> of matching locations.
     */
    virtual std::vector<Location> findByType(LocationType type) const = 0;

    /**
     * Saves a location into repository storage.
     * Rejects duplicates if locationId already exists.
     * @param location Location entity to persist.
     * @return true if successfully saved, false if rejected as duplicate.
     */
    virtual bool save(const Location& location) = 0;
};

/**
 * ============================================================================
 * OOP Concept: Concrete Implementation & Encapsulation
 * - Implements ILocationRepository contract with in-memory storage structures.
 *
 * DSA Demonstrations & Canonical Terminology:
 * - HashMap (itemMapById: std::unordered_map<std::string, Location>):
 *   Enables average O(1) key-based retrieval by ID.
 * - Dynamic Array (itemList: std::vector<Location>):
 *   Maintains insertion order and sequential iteration with O(1) amortized append.
 * - HashSet (uniqueIdSet: std::unordered_set<std::string>):
 *   Enforces uniqueness of primary keys with average O(1) lookup to reject duplicates.
 *
 * Time Complexities:
 * - findById(id): Average O(1), Worst-case O(N) under high hash collision.
 * - save(location): Average O(1) verification and insertion; O(1) amortized append.
 * - findAll(): O(N) linear time to copy elements.
 * - findByType(type): O(N) linear iteration filtering matching items.
 * ============================================================================
 */
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

    // Diagnostics / sizing helpers
    size_t size() const;
    void clear();
};
