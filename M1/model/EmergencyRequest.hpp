#pragma once
#include <string>
struct IncidentLocation
{
    std::string locationId;
    std::string address;
    double latitude = 0.0;
    double longitude = 0.0;
};
enum class EmergencyStatus
{
    PENDING,
    ASSIGNED,
    ACCEPTED,
    EN_ROUTE,
    COMPLETED,
    CANCELLED
};
enum class EmergencyPriority
{
    LOW = 1,
    NORMAL = 2,
    HIGH = 3,
    CRITICAL = 4
};
enum class EmergencyServiceType
{
    AMBULANCE,
    AIR_AMBULANCE,
    POLICE,
    FIRE
};
class EmergencyRequest
{
public:
    EmergencyRequest(const std::string &id,
                     const std::string &userId,
                     const IncidentLocation &location,
                     EmergencyServiceType serviceType,
                     EmergencyPriority priority,
                     const std::string &description = "")
        : id(id),
          userId(userId),
          location(location),
          serviceType(serviceType),
          priority(priority),
          description(description)
    {
    }
    const std::string &getId() const
    {
        return id;
    }
    const std::string &getUserId() const
    {
        return userId;
    }
    const IncidentLocation &getLocation() const
    {
        return location;
    }
    EmergencyServiceType getServiceType() const
    {
        return serviceType;
    }
    EmergencyPriority getPriority() const
    {
        return priority;
    }
    EmergencyStatus getStatus() const
    {
        return status;
    }
    const std::string &getDescription() const
    {
        return description;
    }
    void setStatus(EmergencyStatus newStatus)
    {
        status = newStatus;
    }
private:
    std::string id;
    std::string userId;
    IncidentLocation location;
    EmergencyServiceType serviceType;
    EmergencyPriority priority;
    std::string description;
    EmergencyStatus status = EmergencyStatus::PENDING;
};