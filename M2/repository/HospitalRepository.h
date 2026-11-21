#ifndef M2_REPOSITORY_HOSPITAL_REPOSITORY_H
#define M2_REPOSITORY_HOSPITAL_REPOSITORY_H

#include "IHospitalRepository.h"
#include <unordered_map>
#include <mutex>

class HospitalRepository : public IHospitalRepository {
private:
    std::unordered_map<std::string, Hospital> storage;
    mutable std::mutex repoMutex;

public:
    HospitalRepository();

    bool save(const Hospital& hospital) override;
    std::optional<Hospital> findById(const std::string& id) const override;
    std::vector<Hospital> findAll() const override;
    size_t count() const override;

    // Seeds default Dehradun hospitals matching the live prototype
    void seedDehradunHospitals();
};

#endif // M2_REPOSITORY_HOSPITAL_REPOSITORY_H
