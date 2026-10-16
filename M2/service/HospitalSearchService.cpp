#include "HospitalSearchService.hpp"
#include <algorithm>
#include <cctype>

namespace {
std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
bool containsIgnoreCase(const std::string& value, const std::string& query) {
    return query.empty() || lower(value).find(lower(query)) != std::string::npos;
}
bool hasDepartment(const Hospital& h, const std::string& query) {
    if (query.empty()) return true;
    for (const auto& d : h.getDepartments())
        if (containsIgnoreCase(d.getName(), query)) return true;
    return false;
}
bool hasMedicalService(const Hospital& h, const std::string& query) {
    if (query.empty()) return true;
    for (const auto& s : h.getMedicalServices())
        if (s.isAvailable() && containsIgnoreCase(s.getName(), query)) return true;
    return false;
}
}

std::vector<Hospital> HospitalSearchService::filterHospitals(const HospitalSearchCriteria& c) const {
    auto itemList = repository_.findAll();
    itemList.erase(std::remove_if(itemList.begin(), itemList.end(), [&](const Hospital& h) {
        return !containsIgnoreCase(h.getName(), c.nameContains) ||
               !hasDepartment(h, c.departmentName) ||
               !hasMedicalService(h, c.medicalServiceName) ||
               (c.requireAnyAvailableBed && !h.getBedAvailability().hasAnyBed());
    }), itemList.end());
    const auto byName = [](const Hospital& a, const Hospital& b) { return lower(a.getName()) < lower(b.getName()); };
    if (c.sortBy == HospitalSortBy::RATING_DESC) {
        std::sort(itemList.begin(), itemList.end(), [&](const Hospital& a, const Hospital& b) {
            if (a.getRating() != b.getRating()) return a.getRating() > b.getRating();
            return byName(a, b);
        });
    } else if (c.sortBy == HospitalSortBy::AVAILABLE_BEDS_DESC) {
        std::sort(itemList.begin(), itemList.end(), [&](const Hospital& a, const Hospital& b) {
            const int bedsA = a.getBedAvailability().getGeneralBeds() + a.getBedAvailability().getIcuBeds();
            const int bedsB = b.getBedAvailability().getGeneralBeds() + b.getBedAvailability().getIcuBeds();
            if (bedsA != bedsB) return bedsA > bedsB;
            return byName(a, b);
        });
    } else {
        std::sort(itemList.begin(), itemList.end(), byName);
    }
    return itemList;
}

const Hospital* HospitalSearchService::findHospitalById(const std::string& id) const {
    return repository_.findHospitalById(id);
}

std::vector<Doctor> HospitalSearchService::searchDoctorsByDepartment(const std::string& hospitalId,
                                                                      const std::string& departmentId) const {
    std::vector<Doctor> itemList;
    const Hospital* hospital = findHospitalById(hospitalId);
    if (!hospital) return itemList;
    for (const auto& doctor : hospital->getDoctors())
        if (doctor.getDepartmentId() == departmentId) itemList.push_back(doctor);
    std::sort(itemList.begin(), itemList.end(), [](const Doctor& a, const Doctor& b) {
        return a.getName() < b.getName();
    });
    return itemList;
}
