#pragma once
#include <string>
#include <utility>

class Department {
public:
    Department(std::string id, std::string name, std::string description = "")
        : id_(std::move(id)), name_(std::move(name)), description_(std::move(description)) {}
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
private:
    std::string id_, name_, description_;
};
