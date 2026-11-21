#ifndef M2_MODEL_HOSPITAL_H
#define M2_MODEL_HOSPITAL_H

#include "BedAvailability.h"
#include "Department.h"
#include <string>
#include <vector>

class Hospital {
private:
    std::string id;
    std::string name;
    std::string address;
    std::string phone;
    std::string emergencyService;
    BedAvailability beds;
    std::vector<std::string> specialties;
    std::vector<Department> departments;
    double latitude;
    double longitude;
    double rating;

public:
    Hospital();
    Hospital(const std::string& id,
             const std::string& name,
             const std::string& address,
             const std::string& phone,
             const std::string& emergency,
             const BedAvailability& beds,
             double lat,
             double lng,
             double rating = 4.5);

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getAddress() const { return address; }
    const std::string& getPhone() const { return phone; }
    const std::string& getEmergencyService() const { return emergencyService; }
    const BedAvailability& getBeds() const { return beds; }
    const std::vector<std::string>& getSpecialties() const { return specialties; }
    const std::vector<Department>& getDepartments() const { return departments; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }
    double getRating() const { return rating; }

    void addSpecialty(const std::string& spec) { specialties.push_back(spec); }
    void addDepartment(const Department& dept) { departments.push_back(dept); }
    bool hasSpecialty(const std::string& spec) const;

    std::string toJson() const;
};

#endif // M2_MODEL_HOSPITAL_H
