#include "../repository/HospitalRepository.hpp"
#include "../service/HospitalSearchService.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

void displayHospitals(const vector<Hospital>& hospitals)
{
    if (hospitals.empty())
    {
        cout << "\nNo hospitals found.\n";
        return;
    }

    cout << "\n";
    cout << left
         << setw(12) << "ID"
         << setw(30) << "Hospital Name"
         << setw(10) << "Rating"
         << setw(15) << "General Beds"
         << setw(12) << "ICU Beds"
         << "\n";

    cout << string(79, '-') << "\n";

    for (const Hospital& hospital : hospitals)
    {
        cout << left
             << setw(12) << hospital.getId()
             << setw(30) << hospital.getName()
             << setw(10) << hospital.getRating()
             << setw(15)
             << hospital.getBedAvailability().getGeneralBeds()
             << setw(12)
             << hospital.getBedAvailability().getIcuBeds()
             << "\n";
    }
}

void displayHospitalDetails(const Hospital* hospital)
{
    if (hospital == nullptr)
    {
        cout << "\nHospital not found.\n";
        return;
    }

    cout << "\n========== HOSPITAL DETAILS ==========\n";
    cout << "ID             : " << hospital->getId() << "\n";
    cout << "Name           : " << hospital->getName() << "\n";
    cout << "Address        : " << hospital->getAddress() << "\n";
    cout << "Contact        : " << hospital->getContact() << "\n";
    cout << "Rating         : " << hospital->getRating() << "\n";
    cout << "Location ID    : " << hospital->getLocationId() << "\n";

    cout << "General Beds   : "
         << hospital->getBedAvailability().getGeneralBeds() << "\n";

    cout << "ICU Beds       : "
         << hospital->getBedAvailability().getIcuBeds() << "\n";

    cout << "Any Bed Available: "
         << (hospital->getBedAvailability().hasAnyBed() ? "Yes" : "No")
         << "\n";

    cout << "\nDepartments:\n";

    if (hospital->getDepartments().empty())
    {
        cout << "No department information available.\n";
    }
    else
    {
        for (const Department& department : hospital->getDepartments())
        {
            cout << "- " << department.getName() << "\n";
        }
    }

    cout << "\nDoctors:\n";

    if (hospital->getDoctors().empty())
    {
        cout << "No doctor information available.\n";
    }
    else
    {
        for (const Doctor& doctor : hospital->getDoctors())
        {
            cout << "- " << doctor.getName()
                 << " | Specialty: " << doctor.getSpecialty()
                 << " | Available: "
                 << (doctor.isAvailable() ? "Yes" : "No")
                 << "\n";
        }
    }

    cout << "\nMedical Services:\n";

    if (hospital->getMedicalServices().empty())
    {
        cout << "No medical service information available.\n";
    }
    else
    {
        for (const MedicalService& service : hospital->getMedicalServices())
        {
            cout << "- " << service.getName()
                 << " | Available: "
                 << (service.isAvailable() ? "Yes" : "No")
                 << "\n";
        }
    }

    cout << "======================================\n";
}

int main()
{
    HospitalRepository hospitals;

    /*
       Load hospital records from CSV.
       Run the program with M2 as the working directory,
       so the relative path "data/hospitals.csv" resolves correctly.
    */

    if (!hospitals.loadFromCSV("data/hospitals.csv"))
    {
        cerr << "Error: Could not load hospital data from CSV.\n";
        cerr << "Check that M2/data/hospitals.csv exists and is readable.\n";
        return 1;
    }

    HospitalSearchService search(hospitals);

    int choice;

    do
    {
        cout << "\n";
        cout << "============================================\n";
        cout << " RESQ DEHRADUN - HOSPITAL DISCOVERY SYSTEM\n";
        cout << "============================================\n";
        cout << "1. Display all hospitals\n";
        cout << "2. Search hospital by ID\n";
        cout << "3. Search hospitals by name\n";
        cout << "4. Search hospitals by department\n";
        cout << "5. Search hospitals with available beds\n";
        cout << "6. Sort hospitals by rating\n";
        cout << "7. Sort hospitals by available beds\n";
        cout << "8. Search doctors by department\n";
        cout << "9. Add a hospital\n";
        cout << "10. Update hospital bed availability\n";
        cout << "11. Save hospital data to CSV\n";
        cout << "0. Exit\n";
        cout << "============================================\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cout << "\nInvalid input. Please enter a number.\n";
            return 1;
        }

        cin.ignore(10000, '\n');

        switch (choice)
        {
            case 1:
            {
                displayHospitals(hospitals.findAll());
                break;
            }

            case 2:
            {
                string id;

                cout << "Enter hospital ID: ";
                getline(cin, id);

                const Hospital* hospital = search.findHospitalById(id);

                displayHospitalDetails(hospital);
                break;
            }

            case 3:
            {
                string name;

                cout << "Enter hospital name or part of its name: ";
                getline(cin, name);

                HospitalSearchCriteria criteria;
                criteria.nameContains = name;
                criteria.sortBy = HospitalSortBy::NAME;

                displayHospitals(search.filterHospitals(criteria));
                break;
            }

            case 4:
            {
                string department;

                cout << "Enter department name: ";
                getline(cin, department);

                HospitalSearchCriteria criteria;
                criteria.departmentName = department;
                criteria.sortBy = HospitalSortBy::NAME;

                displayHospitals(search.filterHospitals(criteria));
                break;
            }

            case 5:
            {
                HospitalSearchCriteria criteria;
                criteria.requireAnyAvailableBed = true;
                criteria.sortBy = HospitalSortBy::AVAILABLE_BEDS_DESC;

                displayHospitals(search.filterHospitals(criteria));
                break;
            }

            case 6:
            {
                HospitalSearchCriteria criteria;
                criteria.sortBy = HospitalSortBy::RATING_DESC;

                displayHospitals(search.filterHospitals(criteria));
                break;
            }

            case 7:
            {
                HospitalSearchCriteria criteria;
                criteria.sortBy = HospitalSortBy::AVAILABLE_BEDS_DESC;

                displayHospitals(search.filterHospitals(criteria));
                break;
            }

            case 8:
            {
                string hospitalId;
                string departmentId;

                cout << "Enter hospital ID: ";
                getline(cin, hospitalId);

                cout << "Enter department ID: ";
                getline(cin, departmentId);

                vector<Doctor> doctors =
                    search.searchDoctorsByDepartment(hospitalId, departmentId);

                if (doctors.empty())
                {
                    cout << "\nNo doctors found for the given hospital "
                         << "and department.\n";
                }
                else
                {
                    cout << "\nDoctors found:\n";

                    for (const Doctor& doctor : doctors)
                    {
                        cout << "ID: " << doctor.getId()
                             << " | Name: " << doctor.getName()
                             << " | Specialty: " << doctor.getSpecialty()
                             << " | Available: "
                             << (doctor.isAvailable() ? "Yes" : "No")
                             << "\n";
                    }
                }

                break;
            }

            case 9:
            {
                string id, name, address, contact, locationId;
                double rating;
                int generalBeds, icuBeds;

                cout << "Enter hospital ID: ";
                getline(cin, id);

                cout << "Enter hospital name: ";
                getline(cin, name);

                cout << "Enter address: ";
                getline(cin, address);

                cout << "Enter contact number: ";
                getline(cin, contact);

                cout << "Enter location ID: ";
                getline(cin, locationId);

                cout << "Enter rating (0-5): ";

                if (!(cin >> rating))
                {
                    cout << "Invalid rating input.\n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    break;
                }

                cout << "Enter general bed count: ";

                if (!(cin >> generalBeds))
                {
                    cout << "Invalid general bed count.\n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    break;
                }

                cout << "Enter ICU bed count: ";

                if (!(cin >> icuBeds))
                {
                    cout << "Invalid ICU bed count.\n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    break;
                }

                cin.ignore(10000, '\n');

                if (rating < 0 || rating > 5 ||
                    generalBeds < 0 || icuBeds < 0)
                {
                    cout << "\nInvalid rating or bed count.\n";
                    break;
                }

                Hospital hospital(
                    id,
                    name,
                    address,
                    contact,
                    rating,
                    locationId,
                    BedAvailability(generalBeds, icuBeds)
                );

                if (hospitals.add(hospital))
                {
                    cout << "\nHospital added successfully in memory.\n";
                    cout << "Choose option 11 to save changes to CSV.\n";
                }
                else
                {
                    cout << "\nCould not add hospital. "
                         << "The ID may already exist or be empty.\n";
                }

                break;
            }

            case 10:
            {
                string id;
                int generalBeds, icuBeds;

                cout << "Enter hospital ID: ";
                getline(cin, id);

                const Hospital* existing = hospitals.findHospitalById(id);

                if (existing == nullptr)
                {
                    cout << "\nHospital not found.\n";
                    break;
                }

                cout << "Enter updated general bed count: ";

                if (!(cin >> generalBeds))
                {
                    cout << "Invalid general bed count.\n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    break;
                }

                cout << "Enter updated ICU bed count: ";

                if (!(cin >> icuBeds))
                {
                    cout << "Invalid ICU bed count.\n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    break;
                }

                cin.ignore(10000, '\n');

                if (generalBeds < 0 || icuBeds < 0)
                {
                    cout << "\nBed counts cannot be negative.\n";
                    break;
                }

                Hospital updated = *existing;

                updated.setBedAvailability(
                    BedAvailability(generalBeds, icuBeds)
                );

                if (hospitals.update(updated))
                {
                    cout << "\nBed availability updated in memory.\n";
                    cout << "Choose option 11 to save changes to CSV.\n";
                }
                else
                {
                    cout << "\nCould not update hospital.\n";
                }

                break;
            }

            case 11:
            {
                if (hospitals.saveToCSV("data/hospitals.csv"))
                {
                    cout << "\nHospital data saved to CSV successfully.\n";
                }
                else
                {
                    cout << "\nError: Could not save hospital data to CSV.\n";
                }

                break;
            }

            case 0:
            {
                cout << "\nExiting ResQ Hospital Discovery System.\n";
                cout << "Remember to save changes before exiting.\n";
                break;
            }

            default:
            {
                cout << "\nInvalid choice. Please try again.\n";
            }
        }

    } while (choice != 0);

    return 0;
}