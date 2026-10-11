#pragma once
#include <string>
#include <utility>

class Doctor {
public:
    Doctor(std::string id, std::string name, std::string departmentId, std::string specialty, bool available = true)
        : id_(std::move(id)), name_(std::move(name)), departmentId_(std::move(departmentId)),
          specialty_(std::move(specialty)), available_(available) {}
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDepartmentId() const { return departmentId_; }
    const std::string& getSpecialty() const { return specialty_; }
    bool isAvailable() const { return available_; }
    void setAvailable(bool available) { available_ = available; }
private:
    std::string id_, name_, departmentId_, specialty_;
    bool available_;
};
