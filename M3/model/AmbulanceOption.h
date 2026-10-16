#pragma once

#include "model/TransportOption.h"

class AmbulanceOption : public TransportOption {
public:
    AmbulanceOption(const std::string& optionId = "TRN-001",
                    const std::string& displayName = "Ambulance",
                    double averageSpeedKmph = 35.0,
                    double costPerKm = 0.0,
                    double baseFare = 0.0,
                    double maxPracticalDistanceKm = 300.0);

    double speedFor(RoadType roadType) const override;
    double fixedOverheadMinutes() const override;
    bool isSuitableFor(double totalDistanceKm) const override;
    std::string unsuitableReason(double totalDistanceKm) const override;
};