#pragma once

#include "model/TransportOption.h"

class CarOption : public TransportOption {
public:
    static constexpr double DEMO_HIGHWAY_SPEED_KMPH = 60.0;
    static constexpr double DEMO_MAIN_ROAD_SPEED_KMPH = 30.0;
    static constexpr double DEMO_CITY_STREET_SPEED_KMPH = 18.0;
    static constexpr double DEMO_HILL_ROAD_SPEED_KMPH = 22.0;
    static constexpr double DEMO_OVERHEAD_MINUTES = 3.0;

    CarOption(const std::string& optionId = "TRN-003",
              const std::string& displayName = "Car / Cab",
              double averageSpeedKmph = 32.5,
              double costPerKm = 12.0,
              double baseFare = 50.0,
              double maxPracticalDistanceKm = 300.0);

    double speedFor(RoadType roadType) const override;
    double fixedOverheadMinutes() const override;
    bool isSuitableFor(double totalDistanceKm) const override;
    std::string unsuitableReason(double totalDistanceKm) const override;
};
