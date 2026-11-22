#ifndef M3_REPOSITORY_LOCATION_REPOSITORY_H
#define M3_REPOSITORY_LOCATION_REPOSITORY_H

#include "ILocationRepository.h"
#include <unordered_map>
#include <mutex>

class LocationRepository : public ILocationRepository {
private:
    std::unordered_map<std::string, Location> storage;
    mutable std::mutex repoMutex;

public:
    LocationRepository();

    bool save(const Location& location) override;
    std::optional<Location> findById(const std::string& id) const override;
    std::vector<Location> findAll() const override;
    std::vector<Location> findByType(LocationType type) const override;
    size_t count() const override;
    void clear() override;
};

#endif // M3_REPOSITORY_LOCATION_REPOSITORY_H
