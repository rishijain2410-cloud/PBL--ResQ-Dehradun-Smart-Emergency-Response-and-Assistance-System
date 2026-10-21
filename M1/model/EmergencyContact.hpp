#pragma once
#include <string>
class EmergencyContact
{
public:
    EmergencyContact(const std::string &id,
                     const std::string &userId,
                     const std::string &name,
                     const std::string &phone,
                     const std::string &relationship)
        : id(id),
          userId(userId),
          name(name),
          phone(phone),
          relationship(relationship)
    {
    }
    std::string getId() const
    {
        return id;
    }
    std::string getUserId() const
    {
        return userId;
    }
    std::string getName() const
    {
        return name;
    }
    std::string getPhone() const
    {
        return phone;
    }
    std::string getRelationship() const
    {
        return relationship;
    }
private:
    std::string id;
    std::string userId;
    std::string name;
    std::string phone;
    std::string relationship;
};
