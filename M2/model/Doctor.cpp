#include "Doctor.h"
#include <sstream>

Doctor::Doctor() : doctorId(""), name(""), department(""), specialization(""), onDuty(false) {}

Doctor::Doctor(const std::string& id, const std::string& n, const std::string& d, const std::string& s, bool duty)
    : doctorId(id), name(n), department(d), specialization(s), onDuty(duty) {}

std::string Doctor::toJson() const {
    std::ostringstream ss;
    ss << "{\"id\":\"" << doctorId << "\",\"name\":\"" << name
       << "\",\"department\":\"" << department
       << "\",\"specialization\":\"" << specialization
       << "\",\"onDuty\":" << (onDuty ? "true" : "false") << "}";
    return ss.str();
}
