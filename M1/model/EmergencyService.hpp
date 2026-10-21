#pragma once
#include "EmergencyRequest.hpp"
#include <string>
class EmergencyService
{
public:
    EmergencyService(const std::string &id,
                     const std::string &name,
                     const std::string &phone,
                     bool available)
        : id(id),
          name(name),
          phone(phone),
          available(available)
    {
    }
    virtual ~EmergencyService()
    {
    }
    const std::string &getId() const
    {
        return id;
    }
    const std::string &getName() const
    {
        return name;
    }
    const std::string &getPhone() const
    {
        return phone;
    }
    bool isAvailable() const
    {
        return available;
    }
    void setAvailable(bool newAvailability)
    {
        available = newAvailability;
    }
    virtual EmergencyServiceType getType() const = 0;
    virtual std::string getDisplayDetails() const = 0;

private:
    std::string id;
    std::string name;
    std::string phone;
    bool available;
};
