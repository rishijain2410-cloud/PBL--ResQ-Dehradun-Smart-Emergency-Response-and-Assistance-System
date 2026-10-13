#include "repository/LocationRepository.h"

using namespace std;

optional<Location> InMemoryLocationRepository::findById(const string& id) const {
    // Average O(1) hash map lookup
    auto it = itemMapById.find(id);
    if (it != itemMapById.end()) {
        return it->second;
    }
    return nullopt;
}

vector<Location> InMemoryLocationRepository::findAll() const {
    // Returns full list in insertion order
    return itemList;
}

vector<Location> InMemoryLocationRepository::findByType(LocationType type) const {
    vector<Location> filtered;
    filtered.reserve(itemList.size() / 4); // Heuristic initial capacity
    for (const auto& location : itemList) {
        if (location.getLocationType() == type) {
            filtered.push_back(location);
        }
    }
    return filtered;
}

bool InMemoryLocationRepository::save(const Location& location) {
    const string& id = location.getLocationId();

    // Check duplicate using HashSet in average O(1)
    if (uniqueIdSet.find(id) != uniqueIdSet.end()) {
        return false; // Duplicate detected; rejected
    }

    // Persist in hash set, hash map, and ordered vector
    uniqueIdSet.insert(id);
    itemMapById.emplace(id, location);
    itemList.push_back(location);
    return true;
}

size_t InMemoryLocationRepository::size() const {
    return itemList.size();
}

void InMemoryLocationRepository::clear() {
    uniqueIdSet.clear();
    itemMapById.clear();
    itemList.clear();
}
