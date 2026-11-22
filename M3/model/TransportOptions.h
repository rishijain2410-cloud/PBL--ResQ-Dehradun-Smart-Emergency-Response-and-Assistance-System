#ifndef M3_MODEL_TRANSPORT_OPTIONS_H
#define M3_MODEL_TRANSPORT_OPTIONS_H

#include "../structure/RoadEdge.h"
#include "../dto/RouteSegment.h"
#include <string>
#include <vector>

class TransportOption {
protected:
    std::string modeName;
    double baseSpeedKmH;
    double costPerKm;
    double baseFare;
    double maxDistanceKm;

public:
    TransportOption(const std::string& name, double speed, double perKm, double base, double maxDist)
        : modeName(name), baseSpeedKmH(speed), costPerKm(perKm), baseFare(base), maxDistanceKm(maxDist) {}
    virtual ~TransportOption() = default;

    const std::string& getModeName() const { return modeName; }
    double getBaseSpeedKmH() const { return baseSpeedKmH; }
    double getCostPerKm() const { return costPerKm; }
    double getBaseFare() const { return baseFare; }
    double getMaxDistanceKm() const { return maxDistanceKm; }

    virtual double getSpeedForRoadType(RoadType type) const = 0;

    virtual double calculateTravelTimeMinutes(const std::vector<RouteSegment>& segments) const {
        double totalMinutes = 0.0;
        for (const auto& seg : segments) {
            double speed = getSpeedForRoadType(seg.roadType);
            if (speed <= 0.0) speed = baseSpeedKmH;
            double hours = seg.distanceKm / speed;
            totalMinutes += (hours * 60.0);
        }
        return totalMinutes;
    }

    virtual double calculateCost(double totalDistanceKm) const {
        return baseFare + (costPerKm * totalDistanceKm);
    }

    virtual bool isSuitable(double distanceKm) const {
        return distanceKm <= maxDistanceKm;
    }
};

class AmbulanceOption : public TransportOption {
public:
    AmbulanceOption() : TransportOption("Ambulance", 60.0, 12.0, 50.0, 150.0) {}

    double getSpeedForRoadType(RoadType type) const override {
        switch (type) {
            case RoadType::HIGHWAY: return 75.0;
            case RoadType::MAIN_ROAD: return 55.0;
            case RoadType::CITY_STREET: return 45.0;
            case RoadType::HILL_ROAD: return 30.0;
            default: return 50.0;
        }
    }
};

class TwoWheelerOption : public TransportOption {
public:
    TwoWheelerOption() : TransportOption("Two-Wheeler", 45.0, 5.0, 10.0, 50.0) {}

    double getSpeedForRoadType(RoadType type) const override {
        switch (type) {
            case RoadType::HIGHWAY: return 50.0;
            case RoadType::MAIN_ROAD: return 42.0;
            case RoadType::CITY_STREET: return 38.0;
            case RoadType::HILL_ROAD: return 25.0;
            default: return 40.0;
        }
    }
};

class PublicTransitOption : public TransportOption {
public:
    PublicTransitOption() : TransportOption("Public Transit (Bus)", 30.0, 3.0, 15.0, 100.0) {}

    double getSpeedForRoadType(RoadType type) const override {
        switch (type) {
            case RoadType::HIGHWAY: return 42.0;
            case RoadType::MAIN_ROAD: return 30.0;
            case RoadType::CITY_STREET: return 22.0;
            case RoadType::HILL_ROAD: return 18.0;
            default: return 25.0;
        }
    }
};

class WalkingOption : public TransportOption {
public:
    WalkingOption() : TransportOption("Walking", 5.0, 0.0, 0.0, 5.0) {}

    double getSpeedForRoadType(RoadType type) const override {
        switch (type) {
            case RoadType::HILL_ROAD: return 3.5;
            default: return 5.0;
        }
    }

    double calculateCost(double) const override {
        return 0.0;
    }

    bool isSuitable(double distanceKm) const override {
        return distanceKm <= 5.0; // Walking rejected for distances > 5 km
    }
};

#endif // M3_MODEL_TRANSPORT_OPTIONS_H
