#include "Department.h"
#include <sstream>

Department::Department() : name(""), headDoctor("") {}

Department::Department(const std::string& n, const std::string& head)
    : name(n), headDoctor(head) {}

std::string Department::toJson() const {
    std::ostringstream ss;
    ss << "{\"name\":\"" << name << "\",\"headDoctor\":\"" << headDoctor << "\",\"doctors\":[";
    for (size_t i = 0; i < doctors.size(); ++i) {
        ss << doctors[i].toJson();
        if (i + 1 < doctors.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}
