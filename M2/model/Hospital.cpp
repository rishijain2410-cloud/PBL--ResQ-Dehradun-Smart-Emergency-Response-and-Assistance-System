#include "Hospital.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

Hospital::Hospital()
    : id(""), name(""), address(""), phone(""), emergencyService(""),
      beds(), latitude(0.0), longitude(0.0), rating(4.5) {}

Hospital::Hospital(const std::string& i,
                   const std::string& n,
                   const std::string& addr,
                   const std::string& ph,
                   const std::string& em,
                   const BedAvailability& b,
                   double lat,
                   double lng,
                   double r)
    : id(i), name(n), address(addr), phone(ph), emergencyService(em),
      beds(b), latitude(lat), longitude(lng), rating(r) {}

bool Hospital::hasSpecialty(const std::string& spec) const {
    std::string target = spec;
    std::transform(target.begin(), target.end(), target.begin(), ::tolower);
    for (const auto& s : specialties) {
        std::string sLow = s;
        std::transform(sLow.begin(), sLow.end(), sLow.begin(), ::tolower);
        if (sLow.find(target) != std::string::npos) return true;
    }
    return false;
}

std::string Hospital::toJson() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(5);
    ss << "{"
       << "\"id\":\"" << id << "\","
       << "\"name\":\"" << name << "\","
       << "\"address\":\"" << address << "\","
       << "\"phone\":\"" << phone << "\","
       << "\"emergency\":\"" << emergencyService << "\","
       << "\"beds\":" << beds.toJson() << ","
       << "\"latitude\":" << latitude << ","
       << "\"longitude\":" << longitude << ","
       << "\"rating\":" << rating << ","
       << "\"specialties\":[";
    for (size_t i = 0; i < specialties.size(); ++i) {
        ss << "\"" << specialties[i] << "\"";
        if (i + 1 < specialties.size()) ss << ",";
    }
    ss << "],\"departments\":[";
    for (size_t i = 0; i < departments.size(); ++i) {
        ss << departments[i].toJson();
        if (i + 1 < departments.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}
