#pragma once
#include <string>

enum class HospitalSortBy { NAME, RATING_DESC, AVAILABLE_BEDS_DESC };

struct HospitalSearchCriteria {
    std::string nameContains;
    std::string departmentName;
    std::string medicalServiceName;
    bool requireAnyAvailableBed = false;
    HospitalSortBy sortBy = HospitalSortBy::NAME;
};
