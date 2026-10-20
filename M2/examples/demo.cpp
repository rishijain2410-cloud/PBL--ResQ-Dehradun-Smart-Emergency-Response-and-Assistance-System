
#include "../repository/HospitalRepository.hpp"
#include "../service/HospitalSearchService.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Display hospital details
void displayHospital(const Hospital& hospital)
{
    cout << "\n----------------------------------------\n";
    cout << "Hospital Name: " << hospital.getName() << '\n';
    cout << "Hospital ID: " << hospital.getId() << '\n';
    cout << "Address: " << hospital.getAddress() << '\n';
    cout << "Contact: " << hospital.getContact() << '\n';
    cout << "Rating: " << hospital.getRating() << "/5\n";

    cout << "General Beds: "
         << hospital.getBedAvailability().getGeneralBeds() << '\n';

    cout << "ICU Beds: "
         << hospital.getBedAvailability().getIcuBeds() << '\n';

    cout << "Departments:\n";

    for (const auto& department : hospital.getDepartments())
    {
        cout << "  - " << department.getName() << '\n';
    }

    cout << "Doctors:\n";

    for (const auto& doctor : hospital.getDoctors())
    {
        cout << "  - " << doctor.getName()
             << " | Specialty: " << doctor.getSpecialty()
             << '\n';
    }

    cout << "Medical Services:\n";

    for (const auto& service : hospital.getMedicalServices())
    {
        cout << "  - " << service.getName()
             << " | "
             << (service.isAvailable() ? "Available" : "Unavailable")
             << '\n';
    }

    cout << "----------------------------------------\n";
}

// Display a list of hospitals
void displayHospitals(const vector<Hospital>& hospitals)
{
    if (hospitals.empty())
    {
        cout << "No hospitals found matching the criteria.\n";
        return;
    }

    for (const auto& hospital : hospitals)
    {
        cout << hospital.getName()
             << " | Rating: " << hospital.getRating()
             << " | General Beds: "
             << hospital.getBedAvailability().getGeneralBeds()
             << " | ICU Beds: "
             << hospital.getBedAvailability().getIcuBeds()
             << '\n';
    }
}

int main()
{
    cout << "\n";
    cout << "============================================\n";
    cout << "       RESQ - HOSPITAL DISCOVERY SYSTEM\n";
    cout << "============================================\n";

    // 1. Create repository
    HospitalRepository hospitals;

    // 2. Create departments
    vector<Department> departments1 = {
        Department("DEP-CARD", "Cardiology"),
        Department("DEP-ORTHO", "Orthopedics"),
        Department("DEP-ER", "Emergency Medicine")
    };

    vector<Department> departments2 = {
        Department("DEP-NEURO", "Neurology"),
        Department("DEP-GEN", "General Medicine")
    };

    vector<Department> departments3 = {
        Department("DEP-PED", "Pediatrics"),
        Department("DEP-CARD2", "Cardiology")
    };

    // 3. Create doctors
    vector<Doctor> doctors1 = {
        Doctor("DOC-001", "Dr. Asha Rawat", "DEP-CARD", "Cardiology"),
        Doctor("DOC-002", "Dr. Dev Mehta", "DEP-ORTHO", "Orthopedics")
    };

    vector<Doctor> doctors2 = {
        Doctor("DOC-003", "Dr. Nisha Kapoor", "DEP-NEURO", "Neurology"),
        Doctor("DOC-004", "Dr. Rahul Sharma", "DEP-GEN", "General Medicine")
    };

    vector<Doctor> doctors3 = {
        Doctor("DOC-005", "Dr. Meera Joshi", "DEP-PED", "Pediatrics")
    };

    // 4. Create medical services
    vector<MedicalService> services1 = {
        MedicalService("SRV-CT", "CT Scan", "Diagnostic imaging", true),
        MedicalService("SRV-ER", "Emergency Care", "Emergency department", true),
        MedicalService("SRV-MRI", "MRI", "Magnetic resonance imaging", false)
    };

    vector<MedicalService> services2 = {
        MedicalService("SRV-NEURO", "Neurology Services", "Neurological care", true),
        MedicalService("SRV-PHARM", "Pharmacy", "Medicines and prescriptions", true)
    };

    vector<MedicalService> services3 = {
        MedicalService("SRV-PED", "Pediatric Care", "Child healthcare", true),
        MedicalService("SRV-CT2", "CT Scan", "Diagnostic imaging", true)
    };

    // 5. Add hospitals to repository
    hospitals.add(
        Hospital(
            "HOS-001",
            "Dehradun General Hospital",
            "Race Course, Dehradun",
            "000-000-0001",
            4.2,
            "LOC-001",
            BedAvailability(12, 3),
            departments1,
            doctors1,
            services1
        )
    );

    hospitals.add(
        Hospital(
            "HOS-002",
            "Valley Care Centre",
            "Rajpur Road, Dehradun",
            "000-000-0002",
            4.6,
            "LOC-002",
            BedAvailability(0, 0),
            departments2,
            doctors2,
            services2
        )
    );

    hospitals.add(
        Hospital(
            "HOS-003",
            "Doon Community Hospital",
            "Patel Nagar, Dehradun",
            "000-000-0003",
            3.9,
            "LOC-003",
            BedAvailability(18, 1),
            departments3,
            doctors3,
            services3
        )
    );

    // 6. Create search service
    HospitalSearchService search(hospitals);

    // FEATURE 1: Display all hospitals
    cout << "\n\n========== 1. ALL HOSPITALS ==========\n";

    HospitalSearchCriteria criteria;
    displayHospitals(search.filterHospitals(criteria));

    // FEATURE 2: Search by hospital name
    cout << "\n\n========== 2. SEARCH BY NAME ==========\n";

    criteria = HospitalSearchCriteria{};
    criteria.nameContains = "dehradun";

    displayHospitals(search.filterHospitals(criteria));

    // FEATURE 3: Filter by department
    cout << "\n\n========== 3. FILTER BY DEPARTMENT ==========\n";

    criteria = HospitalSearchCriteria{};
    criteria.departmentName = "cardio";

    displayHospitals(search.filterHospitals(criteria));

    // FEATURE 4: Filter by medical service
    cout << "\n\n========== 4. FILTER BY MEDICAL SERVICE ==========\n";

    criteria = HospitalSearchCriteria{};
    criteria.medicalServiceName = "CT Scan";

    displayHospitals(search.filterHospitals(criteria));

    // FEATURE 5: Hospitals with available beds
    cout << "\n\n========== 5. HOSPITALS WITH AVAILABLE BEDS ==========\n";

    criteria = HospitalSearchCriteria{};
    criteria.requireAnyAvailableBed = true;

    displayHospitals(search.filterHospitals(criteria));

    // FEATURE 6: Sort by rating
    cout << "\n\n========== 6. SORT BY RATING (HIGH TO LOW) ==========\n";

    criteria = HospitalSearchCriteria{};
    criteria.sortBy = HospitalSortBy::RATING_DESC;

    displayHospitals(search.filterHospitals(criteria));

    // FEATURE 7: Sort by available beds
    cout << "\n\n========== 7. SORT BY AVAILABLE BEDS ==========\n";

    criteria = HospitalSearchCriteria{};
    criteria.sortBy = HospitalSortBy::AVAILABLE_BEDS_DESC;

    displayHospitals(search.filterHospitals(criteria));

    // FEATURE 8: Search doctors by department
    cout << "\n\n========== 8. DOCTORS BY DEPARTMENT ==========\n";

    vector<Doctor> cardiologists =
        search.searchDoctorsByDepartment("HOS-001", "DEP-CARD");

    if (cardiologists.empty())
    {
        cout << "No doctors found for this department.\n";
    }
    else
    {
        for (const auto& doctor : cardiologists)
        {
            cout << doctor.getName()
                 << " | Specialty: " << doctor.getSpecialty()
                 << '\n';
        }
    }

    // FEATURE 9: Find hospital by ID
    cout << "\n\n========== 9. SEARCH HOSPITAL BY ID ==========\n";

    const Hospital* selected = search.findHospitalById("HOS-001");

    if (selected != nullptr)
    {
        displayHospital(*selected);
    }
    else
    {
        cout << "Hospital not found.\n";
    }

    // FEATURE 10: Handle invalid hospital ID
    cout << "\n\n========== 10. INVALID HOSPITAL ID ==========\n";

    const Hospital* missing = search.findHospitalById("HOS-999");

    if (missing == nullptr)
    {
        cout << "Hospital not found for ID HOS-999.\n";
    }

    // FEATURE 11: Search with no matching results
    cout << "\n\n========== 11. NO MATCHING RESULTS ==========\n";

    criteria = HospitalSearchCriteria{};
    criteria.nameContains = "Nonexistent Hospital";

    displayHospitals(search.filterHospitals(criteria));

    

    return 0;
}