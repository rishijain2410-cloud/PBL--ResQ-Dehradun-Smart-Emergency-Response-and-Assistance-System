#pragma once
#include <string>
#include <utility>

class MedicalService {
public:
    MedicalService(std::string id, std::string name, std::string description = "", bool available = true)
        : id_(std::move(id)), name_(std::move(name)), description_(std::move(description)), available_(available) {}
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    bool isAvailable() const { return available_; }
    void setAvailable(bool available) { available_ = available; }
private:
    std::string id_, name_, description_;
    bool available_;
};
