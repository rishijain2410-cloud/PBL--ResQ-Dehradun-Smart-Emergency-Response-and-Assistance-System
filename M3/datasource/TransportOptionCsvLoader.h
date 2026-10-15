#pragma once

#include "model/TransportOption.h"

#include <vector>
#include <string>
#include <memory>
#include <istream>

class TransportOptionCsvLoader {
public:
    static std::shared_ptr<TransportOption> createTransportOption(
        const std::string& optionId,
        TransportMode mode,
        const std::string& displayName,
        double averageSpeedKmph,
        double costPerKm,
        double baseFare,
        double maxPracticalDistanceKm);

    static std::vector<std::shared_ptr<TransportOption>> loadFromFile(const std::string& filePath);

    static std::vector<std::shared_ptr<TransportOption>> loadFromStream(std::istream& input);

private:
    static std::vector<std::string> parseCsvLine(const std::string& line);
};
