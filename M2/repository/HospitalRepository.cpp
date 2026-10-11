#include "HospitalRepository.hpp"
#include <algorithm>

bool HospitalRepository::add(const Hospital& hospital) {
    if (hospital.getId().empty() || itemMapById.count(hospital.getId())) return false;
    itemMapById.emplace(hospital.getId(), hospital);
    return true;
}
bool HospitalRepository::update(const Hospital& hospital) {
    auto it = itemMapById.find(hospital.getId());
    if (it == itemMapById.end()) return false;
    it->second = hospital;
    return true;
}
const Hospital* HospitalRepository::findHospitalById(const std::string& id) const {
    auto it = itemMapById.find(id);
    return it == itemMapById.end() ? nullptr : &it->second;
}
std::vector<Hospital> HospitalRepository::findAll() const {
    std::vector<Hospital> itemList;
    itemList.reserve(itemMapById.size());
    for (const auto& entry : itemMapById) itemList.push_back(entry.second);
    std::sort(itemList.begin(), itemList.end(), [](const Hospital& a, const Hospital& b) {
        return a.getId() < b.getId();
    });
    return itemList;
}
