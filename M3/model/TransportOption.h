#pragma once

#include "model/TransportMode.h"
#include "structure/RoadEdge.h"
#include "dto/RouteResponse.h"

#include <string>
#include <memory>

class TransportOption {
protected:
    std::string optionId;
    TransportMode mode;
    std::string displayName;
    double averageSpeedKmph;
    double costPerKm;
    double baseFare;
    double maxPracticalDistanceKm;

public:
    TransportOption(const std::string& optionId,
                    TransportMode mode,
                    const std::string& displayName,
                    double averageSpeedKmph,
                    double costPerKm,
                    double baseFare,
                    double maxPracticalDistanceKm);

    virtual ~TransportOption() = default;

    virtual double speedFor(RoadType roadType) const = 0;
    virtual double fixedOverheadMinutes() const = 0;
    virtual bool isSuitableFor(double totalDistanceKm) const = 0;
    virtual std::string unsuitableReason(double totalDistanceKm) const = 0;

    virtual double estimateDurationMinutes(const RouteResponse& route) const;
    virtual double estimateCost(const RouteResponse& route) const;

    const std::string& getOptionId() const;
    TransportMode getMode() const;
    const std::string& getDisplayName() const;
    double getAverageSpeedKmph() const;
    double getCostPerKm() const;
    double getBaseFare() const;
    double getMaxPracticalDistanceKm() const;
};
