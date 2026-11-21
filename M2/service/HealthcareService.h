#ifndef M2_SERVICE_HEALTHCARE_SERVICE_H
#define M2_SERVICE_HEALTHCARE_SERVICE_H

#include "../repository/IHospitalRepository.h"
#include "../dto/HospitalSearchQuery.h"
#include <memory>
#include <vector>

class HealthcareService {
private:
    std::shared_ptr<IHospitalRepository> repository;

public:
    explicit HealthcareService(std::shared_ptr<IHospitalRepository> repo);

    std::vector<Hospital> searchHospitals(const HospitalSearchQuery& query);
    std::optional<Hospital> getHospitalById(const std::string& id);
    std::vector<Hospital> getAllHospitals();
};

#endif // M2_SERVICE_HEALTHCARE_SERVICE_H
