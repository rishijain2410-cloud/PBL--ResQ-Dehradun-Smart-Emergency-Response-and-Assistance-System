#ifndef M2_REPOSITORY_I_HOSPITAL_REPOSITORY_H
#define M2_REPOSITORY_I_HOSPITAL_REPOSITORY_H

#include "../model/Hospital.h"
#include <vector>
#include <string>
#include <optional>

class IHospitalRepository {
public:
    virtual ~IHospitalRepository() = default;

    virtual bool save(const Hospital& hospital) = 0;
    virtual std::optional<Hospital> findById(const std::string& id) const = 0;
    virtual std::vector<Hospital> findAll() const = 0;
    virtual size_t count() const = 0;
};

#endif // M2_REPOSITORY_I_HOSPITAL_REPOSITORY_H
