#pragma once

#include "../model/Hospital.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class HospitalRepository {
public:
    bool loadFromCSV(const std::string& filename);
    bool saveToCSV(const std::string& filename) const;

    bool add(const Hospital& hospital);
    bool update(const Hospital& hospital);

    const Hospital* findHospitalById(const std::string& id) const;
    std::vector<Hospital> findAll() const;

private:
    std::unordered_map<std::string, Hospital> itemMapById;
};