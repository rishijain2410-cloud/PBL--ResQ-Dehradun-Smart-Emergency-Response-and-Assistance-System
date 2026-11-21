#include "HealthcareService.h"
#include <algorithm>

HealthcareService::HealthcareService(std::shared_ptr<IHospitalRepository> repo)
    : repository(repo) {}

std::vector<Hospital> HealthcareService::searchHospitals(const HospitalSearchQuery& query) {
    if (!repository) return {};

    std::vector<Hospital> all = repository->findAll();
    std::vector<Hospital> filtered;

    std::string text = query.textQuery;
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);

    for (const auto& h : all) {
        // 1. Text search
        if (!text.empty()) {
            std::string nameLower = h.getName();
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            std::string addrLower = h.getAddress();
            std::transform(addrLower.begin(), addrLower.end(), addrLower.begin(), ::tolower);

            if (nameLower.find(text) == std::string::npos && addrLower.find(text) == std::string::npos) {
                continue;
            }
        }

        // 2. Specialty filter
        if (!query.specialty.empty() && query.specialty != "all") {
            if (!h.hasSpecialty(query.specialty)) {
                continue;
            }
        }

        // 3. ICU availability filter
        if (query.requireIcu && !h.getBeds().hasIcuCapacity()) {
            continue;
        }

        filtered.push_back(h);
    }

    // Sort results
    if (query.sortBy == HospitalSortBy::AVAILABLE_ICU) {
        std::sort(filtered.begin(), filtered.end(), [](const Hospital& a, const Hospital& b) {
            return a.getBeds().getAvailableIcu() > b.getBeds().getAvailableIcu();
        });
    } else if (query.sortBy == HospitalSortBy::AVAILABLE_BEDS) {
        std::sort(filtered.begin(), filtered.end(), [](const Hospital& a, const Hospital& b) {
            return a.getBeds().getAvailableBeds() > b.getBeds().getAvailableBeds();
        });
    } else if (query.sortBy == HospitalSortBy::RATING) {
        std::sort(filtered.begin(), filtered.end(), [](const Hospital& a, const Hospital& b) {
            return a.getRating() > b.getRating();
        });
    }

    return filtered;
}

std::optional<Hospital> HealthcareService::getHospitalById(const std::string& id) {
    if (!repository) return std::nullopt;
    return repository->findById(id);
}

std::vector<Hospital> HealthcareService::getAllHospitals() {
    if (!repository) return {};
    return repository->findAll();
}
