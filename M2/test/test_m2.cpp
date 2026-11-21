#include "../model/Hospital.h"
#include "../model/BedAvailability.h"
#include "../repository/HospitalRepository.h"
#include "../service/HealthcareService.h"
#include <iostream>
#include <cassert>

static int g_passCount = 0;
static int g_totalCount = 0;

#define M2_ASSERT(cond, msg) do { \
    g_totalCount++; \
    if (cond) { \
        g_passCount++; \
    } else { \
        std::cerr << "  [FAIL] Assertion failed: " << msg << " (" << __LINE__ << ")\n"; \
    } \
} while(0)

void testBedAvailability() {
    std::cout << "[M2 Test] Testing Bed Availability Logic...\n";
    BedAvailability b1(100, 20, 10, 2, 15);
    M2_ASSERT(b1.hasIcuCapacity() == true, "ICU capacity available");
    M2_ASSERT(b1.hasGeneralCapacity() == true, "General bed capacity available");

    BedAvailability b2(100, 0, 10, 0, 5);
    M2_ASSERT(b2.hasIcuCapacity() == false, "No ICU capacity when available is 0");
    M2_ASSERT(b2.hasGeneralCapacity() == false, "No general capacity when available is 0");
}

void testHospitalRepositoryAndSeed() {
    std::cout << "[M2 Test] Testing Hospital Repository & Dehradun Seeds...\n";
    auto repo = std::make_shared<HospitalRepository>();
    M2_ASSERT(repo->count() == 6, "Seeded exactly 6 major Dehradun hospitals");

    auto maxH = repo->findById("LOC-004");
    M2_ASSERT(maxH.has_value(), "Found Max Hospital LOC-004");
    M2_ASSERT(maxH->getName().find("Max Super Speciality") != std::string::npos, "Max Hospital name matches");
    M2_ASSERT(maxH->hasSpecialty("Cardiology") == true, "Max Hospital has Cardiology specialty");
    M2_ASSERT(maxH->hasSpecialty("Dentistry") == false, "Max Hospital does not have Dentistry");
}

void testHealthcareSearchAndSort() {
    std::cout << "[M2 Test] Testing Hospital Search & Sorting...\n";
    auto repo = std::make_shared<HospitalRepository>();
    HealthcareService service(repo);

    // 1. Text search
    HospitalSearchQuery q1;
    q1.textQuery = "patel nagar";
    auto res1 = service.searchHospitals(q1);
    M2_ASSERT(res1.size() == 1, "Found 1 hospital in Patel Nagar (SMIH)");
    M2_ASSERT(res1[0].getId() == "LOC-005", "Result is SMIH LOC-005");

    // 2. Specialty filter: Cardiology
    HospitalSearchQuery q2;
    q2.specialty = "Cardiology";
    auto res2 = service.searchHospitals(q2);
    M2_ASSERT(res2.size() >= 3, "Found 3+ hospitals with Cardiology");

    // 3. Sorting by Available ICU beds
    HospitalSearchQuery q3;
    q3.sortBy = HospitalSortBy::AVAILABLE_ICU;
    auto res3 = service.searchHospitals(q3);
    M2_ASSERT(!res3.empty(), "Hospitals returned for ICU sort");
    for (size_t i = 1; i < res3.size(); ++i) {
        M2_ASSERT(res3[i-1].getBeds().getAvailableIcu() >= res3[i].getBeds().getAvailableIcu(),
                  "Sorted descending by available ICU beds");
    }
}

int main() {
    std::cout << "\n========================================================================\n";
    std::cout << "  MODULE 2 (HEALTHCARE DISCOVERY) TEST SUITE - RISHI JAIN\n";
    std::cout << "========================================================================\n\n";

    testBedAvailability();
    testHospitalRepositoryAndSeed();
    testHealthcareSearchAndSort();

    std::cout << "\n========================================================================\n";
    std::cout << "  M2 TEST SUMMARY: " << g_passCount << " / " << g_totalCount << " PASSED (";
    if (g_passCount == g_totalCount) {
        std::cout << "100% SUCCESS)\n";
    } else {
        std::cout << "FAILURES OCCURRED)\n";
    }
    std::cout << "========================================================================\n\n";

    return (g_passCount == g_totalCount) ? 0 : 1;
}
