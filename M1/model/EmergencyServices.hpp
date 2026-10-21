#pragma once
#include "EmergencyService.hpp"
class AmbulanceService : public EmergencyService
{
public:
    using EmergencyService::EmergencyService;
    EmergencyServiceType getType() const override
    {
        return EmergencyServiceType::AMBULANCE;
    }
    std::string getDisplayDetails() const override
    {
        return getName() + " (ground ambulance)";
    }
};
class AirAmbulanceService : public EmergencyService
{
public:
    using EmergencyService::EmergencyService;
    EmergencyServiceType getType() const override
    {
        return EmergencyServiceType::AIR_AMBULANCE;
    }
    std::string getDisplayDetails() const override
    {
        return getName() + " (air ambulance)";
    }
};
class PoliceService : public EmergencyService
{
public:
    using EmergencyService::EmergencyService;
    EmergencyServiceType getType() const override
    {
        return EmergencyServiceType::POLICE;
    }
    std::string getDisplayDetails() const override
    {
        return getName() + " (police)";
    }
};
class FireService : public EmergencyService
{
public:
    using EmergencyService::EmergencyService;
    EmergencyServiceType getType() const override
    {
        return EmergencyServiceType::FIRE;
    }
    std::string getDisplayDetails() const override
    {
        return getName() + " (fire service)";
    }
};
