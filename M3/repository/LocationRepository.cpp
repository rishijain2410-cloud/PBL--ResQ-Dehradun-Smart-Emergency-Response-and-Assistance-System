#include "LocationRepository.h"

LocationRepository::LocationRepository() {}

bool LocationRepository::save(const Location& location) {
    std::lock_guard<std::mutex> lock(repoMutex);
    storage[location.getId()] = location;
    return true;
}

std::optional<Location> LocationRepository::findById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(repoMutex);
    auto it = storage.find(id);
    if (it != storage.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Location> LocationRepository::findAll() const {
    std::lock_guard<std::mutex> lock(repoMutex);
    std::vector<Location> result;
    result.reserve(storage.size());
    for (const auto& kv : storage) {
        result.push_back(kv.second);
    }
    return result;
}

std::vector<Location> LocationRepository::findByType(LocationType type) const {
    std::lock_guard<std::mutex> lock(repoMutex);
    std::vector<Location> result;
    for (const auto& kv : storage) {
        if (kv.second.getType() == type) {
            result.push_back(kv.second);
        }
    }
    return result;
}

size_t LocationRepository::count() const {
    std::lock_guard<std::mutex> lock(repoMutex);
    return storage.size();
}

void LocationRepository::clear() {
    std::lock_guard<std::mutex> lock(repoMutex);
    storage.clear();
}
