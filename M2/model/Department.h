#ifndef M2_MODEL_DEPARTMENT_H
#define M2_MODEL_DEPARTMENT_H

#include <string>
#include <vector>
#include "Doctor.h"

class Department {
private:
    std::string name;
    std::string headDoctor;
    std::vector<Doctor> doctors;

public:
    Department();
    Department(const std::string& name, const std::string& head);

    const std::string& getName() const { return name; }
    const std::string& getHeadDoctor() const { return headDoctor; }
    const std::vector<Doctor>& getDoctors() const { return doctors; }

    void addDoctor(const Doctor& doc) { doctors.push_back(doc); }
    std::string toJson() const;
};

#endif // M2_MODEL_DEPARTMENT_H
