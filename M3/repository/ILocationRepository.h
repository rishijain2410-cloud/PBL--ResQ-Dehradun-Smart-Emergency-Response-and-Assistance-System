#ifndef M3_REPOSITORY_I_LOCATION_REPOSITORY_H
#define M3_REPOSITORY_I_LOCATION_REPOSITORY_H

#include "../model/Location.h"
#include <vector>
#include <string>
#include <optional>

class ILocationRepository {
public:
    virtual ~ILocationRepository() = default;

    virtual bool save(const Location& location) = 0;
    virtual std::optional<Location> findById(const std::string& id) const = 0;
    virtual std::vector<Location> findAll() const = 0;
    virtual std::vector<Location> findByType(LocationType type) const = 0;
    virtual size_t count() const = 0;
    virtual void clear() = 0;
};

#endif // M3_REPOSITORY_I_LOCATION_REPOSITORY_H
