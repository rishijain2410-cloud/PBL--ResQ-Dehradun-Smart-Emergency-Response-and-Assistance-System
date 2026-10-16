#pragma once

#include <string>
#include <stdexcept>
#include <optional>

enum class TransportMode {
    AMBULANCE,
    CAR
};

std::string toString(TransportMode mode);
std::string transportModeToString(TransportMode mode);
TransportMode fromString(const std::string& modeStr);

std::optional<TransportMode> parseTransportMode(const std::string& modeStr);
