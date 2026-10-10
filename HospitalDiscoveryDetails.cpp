#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Department {
    std::string id;
    std::string name;
};

struct Doctor {
    std::string id;
    std::string name;
    std::string departmentId;
    std::string specialty;
};

struct MedicalService {
    std::string id;
    std::string name;
    bool available = true;
};

class BedAvailability {
public:
    BedAvailability(int generalBeds, int icuBeds)
        : generalBeds_(generalBeds), icuBeds_(icuBeds) {}

    int generalBeds() const { return generalBeds_; }
    int icuBeds() const { return icuBeds_; }
    int totalBeds() const { return generalBeds_ + icuBeds_; }
    bool hasAnyBed() const { return totalBeds() > 0; }

private:
    int generalBeds_;
    int icuBeds_;
};

class Hospital {
public:
    Hospital(std::string id, std::string name, std::string address,
             std::string contact, std::string locationId, double rating,
             BedAvailability beds, std::vector<Department> departments,
             std::vector<Doctor> doctors, std::vector<MedicalService> services)
        : id_(std::move(id)), name_(std::move(name)), address_(std::move(address)),
          contact_(std::move(contact)), locationId_(std::move(locationId)),
          rating_(rating), beds_(beds), departments_(std::move(departments)),
          doctors_(std::move(doctors)), services_(std::move(services)) {}

    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& address() const { return address_; }
    const std::string& contact() const { return contact_; }
    const std::string& locationId() const { return locationId_; }
    double rating() const { return rating_; }
    const BedAvailability& beds() const { return beds_; }
    const std::vector<Department>& departments() const { return departments_; }
    const std::vector<Doctor>& doctors() const { return doctors_; }
    const std::vector<MedicalService>& services() const { return services_; }

private:
    std::string id_;
    std::string name_;
    std::string address_;
    std::string contact_;
    std::string locationId_;
    double rating_;
    BedAvailability beds_;
    std::vector<Department> departments_;
    std::vector<Doctor> doctors_;
    std::vector<MedicalService> services_;
};

class HospitalDirectory {
public:
    bool addHospital(const Hospital& hospital) {
        if (hospital.id().empty() || itemMapById_.count(hospital.id()) != 0) {
            return false;
        }
        itemMapById_.emplace(hospital.id(), hospital);
        return true;
    }

    const Hospital* findHospitalById(const std::string& id) const {
        const auto found = itemMapById_.find(id);
        return found == itemMapById_.end() ? nullptr : &found->second;
    }

    std::vector<Hospital> allHospitals() const {
        std::vector<Hospital> itemList;
        itemList.reserve(itemMapById_.size());
        for (const auto& item : itemMapById_) {
            itemList.push_back(item.second);
        }
        return itemList;
    }

private:
    // Average O(1) lookup by hospital ID.
    std::unordered_map<std::string, Hospital> itemMapById_;
};

struct SearchCriteria {
    std::string name;
    std::string department;
    std::string service;
    bool availableBedsOnly = false;
};

enum class SortOption { NAME, RATING, AVAILABLE_BEDS };

class HospitalSearchService {
public:
    explicit HospitalSearchService(const HospitalDirectory& directory)
        : directory_(directory) {}

    std::vector<Hospital> search(const SearchCriteria& criteria,
                                 SortOption sortBy) const {
        std::vector<Hospital> matches;
        for (const Hospital& hospital : directory_.allHospitals()) {
            if (!matchesCriteria(hospital, criteria)) {
                continue;
            }
            matches.push_back(hospital);
        }

        // Sorting k results takes O(k log k).
        std::sort(matches.begin(), matches.end(), [sortBy](const Hospital& a, const Hospital& b) {
            if (sortBy == SortOption::RATING && a.rating() != b.rating()) {
                return a.rating() > b.rating();
            }
            if (sortBy == SortOption::AVAILABLE_BEDS && a.beds().totalBeds() != b.beds().totalBeds()) {
                return a.beds().totalBeds() > b.beds().totalBeds();
            }
            return lower(a.name()) < lower(b.name());
        });
        return matches;
    }

    const Hospital* findHospitalById(const std::string& id) const {
        return directory_.findHospitalById(id);
    }

private:
    static std::string lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    static bool containsIgnoreCase(const std::string& text, const std::string& query) {
        return query.empty() || lower(text).find(lower(query)) != std::string::npos;
    }

    static bool matchesCriteria(const Hospital& hospital, const SearchCriteria& criteria) {
        if (!containsIgnoreCase(hospital.name(), criteria.name)) {
            return false;
        }
        if (criteria.availableBedsOnly && !hospital.beds().hasAnyBed()) {
            return false;
        }

        if (!criteria.department.empty()) {
            const bool found = std::any_of(hospital.departments().begin(), hospital.departments().end(),
                [&](const Department& department) {
                    return containsIgnoreCase(department.name, criteria.department);
                });
            if (!found) return false;
        }

        if (!criteria.service.empty()) {
            const bool found = std::any_of(hospital.services().begin(), hospital.services().end(),
                [&](const MedicalService& service) {
                    return service.available && containsIgnoreCase(service.name, criteria.service);
                });
            if (!found) return false;
        }
        return true;
    }

    const HospitalDirectory& directory_;
};

void showHospitalDetails(const Hospital& hospital) {
    std::cout << "\n" << hospital.name() << " (" << hospital.id() << ")\n"
              << "Address: " << hospital.address() << "\n"
              << "Contact: " << hospital.contact() << "\n"
              << "Rating: " << hospital.rating() << " / 5\n"
              << "Location ID: " << hospital.locationId() << "\n"
              << "Beds: " << hospital.beds().generalBeds() << " general, "
              << hospital.beds().icuBeds() << " ICU\n\nDepartments:\n";

    for (const auto& department : hospital.departments()) {
        std::cout << "  - " << department.name << "\n";
    }
    std::cout << "Doctors:\n";
    for (const auto& doctor : hospital.doctors()) {
        std::cout << "  - " << doctor.name << " — " << doctor.specialty << "\n";
    }
    std::cout << "Medical services:\n";
    for (const auto& service : hospital.services()) {
        std::cout << "  - " << service.name << (service.available ? " (available)" : " (unavailable)") << "\n";
    }
}

int main() {
    HospitalDirectory directory;
    directory.addHospital(Hospital(
        "HOS-001", "Dehradun General Hospital", "Race Course, Dehradun", "000-000-0001",
        "LOC-001", 4.2, BedAvailability(12, 3),
        {{"DEP-CARD", "Cardiology"}, {"DEP-ER", "Emergency Medicine"}, {"DEP-ORTHO", "Orthopedics"}},
        {{"DOC-001", "Dr. Asha Rawat", "DEP-CARD", "Cardiology"},
         {"DOC-002", "Dr. Dev Mehta", "DEP-ORTHO", "Orthopedics"}},
        {{"SRV-CT", "CT Scan", true}, {"SRV-ER", "Emergency Care", true}, {"SRV-BLOOD", "Blood Bank", true}}
    ));
    directory.addHospital(Hospital(
        "HOS-002", "Valley Care Centre", "Rajpur Road, Dehradun", "000-000-0002",
        "LOC-002", 4.6, BedAvailability(0, 0),
        {{"DEP-CARD-2", "Cardiology"}, {"DEP-NEURO", "Neurology"}},
        {{"DOC-003", "Dr. Nisha Kapoor", "DEP-NEURO", "Neurology"}},
        {{"SRV-CT-2", "CT Scan", true}, {"SRV-MRI", "MRI", true}}
    ));
    directory.addHospital(Hospital(
        "HOS-003", "Doon Community Hospital", "Patel Nagar, Dehradun", "000-000-0003",
        "LOC-003", 3.9, BedAvailability(18, 1),
        {{"DEP-GEN", "General Medicine"}, {"DEP-PED", "Pediatrics"}, {"DEP-ER-2", "Emergency Medicine"}},
        {{"DOC-004", "Dr. Meera Joshi", "DEP-PED", "Pediatrics"}},
        {{"SRV-ER-2", "Emergency Care", true}, {"SRV-WARD", "General Ward", true}, {"SRV-PHARM", "Pharmacy", true}}
    ));

    HospitalSearchService searchService(directory);
    SearchCriteria criteria;
    criteria.department = "cardio";
    criteria.availableBedsOnly = true;

    std::cout << "Hospitals matching Cardiology with available beds:\n";
    const auto results = searchService.search(criteria, SortOption::RATING);
    if (results.empty()) {
        std::cout << "No hospitals found.\n";
        return 0;
    }
    for (const auto& hospital : results) {
        std::cout << "- " << hospital.name() << " | rating " << hospital.rating()
                  << " | available beds " << hospital.beds().totalBeds() << "\n";
    }

    const Hospital* selected = searchService.findHospitalById("HOS-001");
    if (selected != nullptr) {
        showHospitalDetails(*selected);
    }
    return 0;
}
