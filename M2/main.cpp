#include "model/Hospital.h"
#include "repository/HospitalRepository.h"
#include "service/HealthcareService.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "\n========================================================================\n";
    std::cout << "  JEEVAN MARG -- MODULE 2: HEALTHCARE DISCOVERY & BED MANAGER\n";
    std::cout << "  Lead: Rishi Jain (2510010332) | Graphic Era University\n";
    std::cout << "========================================================================\n\n";

    auto repo = std::make_shared<HospitalRepository>();
    HealthcareService service(repo);

    std::cout << "Listing All Registered Dehradun Hospitals & Bed Counts:\n";
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(38) << "Hospital Name"
              << std::setw(12) << "Total Beds"
              << std::setw(12) << "Avail Beds"
              << std::setw(12) << "Avail ICU"
              << std::setw(10) << "Rating\n";
    std::cout << "------------------------------------------------------------------------\n";

    for (const auto& h : service.getAllHospitals()) {
        std::cout << std::left << std::setw(38) << h.getName()
                  << std::setw(12) << h.getBeds().getTotalBeds()
                  << std::setw(12) << h.getBeds().getAvailableBeds()
                  << std::setw(12) << h.getBeds().getAvailableIcu()
                  << std::setw(10) << h.getRating() << "\n";
    }

    std::cout << "\n[Search Query]: Cardiology Department with Available ICU Beds\n";
    HospitalSearchQuery q;
    q.specialty = "Cardiology";
    q.requireIcu = true;
    q.sortBy = HospitalSortBy::AVAILABLE_ICU;

    auto results = service.searchHospitals(q);
    std::cout << "Matches found: " << results.size() << "\n";
    for (const auto& h : results) {
        std::cout << "  * " << h.getName() << " | Available ICU: " << h.getBeds().getAvailableIcu()
                  << " | Contact: " << h.getPhone() << "\n";
    }

    std::cout << "\n";
    return 0;
}
