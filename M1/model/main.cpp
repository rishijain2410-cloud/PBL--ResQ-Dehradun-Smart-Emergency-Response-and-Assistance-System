#include "M1/service/EmergencyManager.hpp"
#include "M1/model/EmergencyServices.hpp"
#include <iostream>
#include <memory>

int main() {
    EmergencyManager manager;

    manager.addEmergencyService(std::make_shared<AmbulanceService>(
        "ESV-001", "Demo Ambulance", "000-000-0101", true));

    IncidentLocation location{
        "LOC-001", "Clock Tower, Dehradun", 0.0, 0.0
    };

    EmergencyRequest request(
        "EMR-001",
        "USR-001",
        location,
        EmergencyServiceType::AMBULANCE,
        EmergencyPriority::HIGH,
        "Demo emergency request"
    );

    if (!manager.createEmergencyRequest(request)) {
        std::cout << "Could not create the request.\n";
        return 1;
    }

    EmergencyRequest next(
        "TEMP", "TEMP", location,
        EmergencyServiceType::AMBULANCE,
        EmergencyPriority::LOW
    );

    if (manager.processNextEmergencyRequest(next)) {
        std::cout << "Processed request: " << next.getId() << '\n';
        std::cout << "Priority: "
                  << static_cast<int>(next.getPriority()) << '\n';
    } else {
        std::cout << "There are no pending requests.\n";
    }

    return 0;
}
