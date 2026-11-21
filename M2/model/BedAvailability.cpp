#include "BedAvailability.h"

BedAvailability::BedAvailability()
    : totalBeds(0), availableBeds(0), totalIcu(0), availableIcu(0), oxygenBeds(0) {}

BedAvailability::BedAvailability(int total, int avail, int icuTot, int icuAvail, int oxy)
    : totalBeds(total), availableBeds(avail), totalIcu(icuTot), availableIcu(icuAvail), oxygenBeds(oxy) {}

std::string BedAvailability::toJson() const {
    std::ostringstream ss;
    ss << "{"
       << "\"totalBeds\":" << totalBeds << ","
       << "\"availableBeds\":" << availableBeds << ","
       << "\"totalIcu\":" << totalIcu << ","
       << "\"availableIcu\":" << availableIcu << ","
       << "\"oxygenBeds\":" << oxygenBeds
       << "}";
    return ss.str();
}
