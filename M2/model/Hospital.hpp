#pragma once
#include "BedAvailability.hpp"
#include "Department.hpp"
#include "Doctor.hpp"
#include "MedicalService.hpp"
#include <string>
#include <utility>
#include <vector>

class Hospital {
public:
    Hospital(std::string id, std::string name, std::string address, std::string contact,
             double rating, std::string locationId, BedAvailability beds,
             std::vector<Department> departments = {}, std::vector<Doctor> doctors = {},
             std::vector<MedicalService> services = {})
        : id_(std::move(id)), name_(std::move(name)), address_(std::move(address)),
          contact_(std::move(contact)), rating_(rating), locationId_(std::move(locationId)),
          beds_(beds), departments_(std::move(departments)), doctors_(std::move(doctors)),
          services_(std::move(services)) {}
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getAddress() const { return address_; }
    const std::string& getContact() const { return contact_; }
    double getRating() const { return rating_; }
    const std::string& getLocationId() const { return locationId_; }
    const BedAvailability& getBedAvailability() const { return beds_; }
    const std::vector<Department>& getDepartments() const { return departments_; }
    const std::vector<Doctor>& getDoctors() const { return doctors_; }
    const std::vector<MedicalService>& getMedicalServices() const { return services_; }
    void setBedAvailability(const BedAvailability& beds) { beds_ = beds; }
private:
    std::string id_, name_, address_, contact_;
    double rating_;
    std::string locationId_;
    BedAvailability beds_;
    std::vector<Department> departments_;
    std::vector<Doctor> doctors_;
    std::vector<MedicalService> services_;
};
