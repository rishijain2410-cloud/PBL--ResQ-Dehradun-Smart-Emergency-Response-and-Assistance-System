#pragma once
#include "../dto/HospitalSearchCriteria.hpp"
#include "../repository/HospitalRepository.hpp"

class HospitalSearchService {
public:
    explicit HospitalSearchService(const HospitalRepository& repository) : repository_(repository) {}
    std::vector<Hospital> filterHospitals(const HospitalSearchCriteria& criteria) const;
    const Hospital* findHospitalById(const std::string& id) const;
    std::vector<Doctor> searchDoctorsByDepartment(const std::string& hospitalId,
                                                   const std::string& departmentId) const;
private:
    const HospitalRepository& repository_;
};
