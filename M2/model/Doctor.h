#ifndef M2_MODEL_DOCTOR_H
#define M2_MODEL_DOCTOR_H

#include <string>

class Doctor {
private:
    std::string doctorId;
    std::string name;
    std::string department;
    std::string specialization;
    bool onDuty;

public:
    Doctor();
    Doctor(const std::string& id, const std::string& name, const std::string& dept, const std::string& spec, bool duty = true);

    const std::string& getId() const { return doctorId; }
    const std::string& getName() const { return name; }
    const std::string& getDepartment() const { return department; }
    const std::string& getSpecialization() const { return specialization; }
    bool isOnDuty() const { return onDuty; }

    std::string toJson() const;
};

#endif // M2_MODEL_DOCTOR_H
