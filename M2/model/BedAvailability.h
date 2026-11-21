#ifndef M2_MODEL_BED_AVAILABILITY_H
#define M2_MODEL_BED_AVAILABILITY_H

#include <string>
#include <sstream>

class BedAvailability {
private:
    int totalBeds;
    int availableBeds;
    int totalIcu;
    int availableIcu;
    int oxygenBeds;

public:
    BedAvailability();
    BedAvailability(int total, int avail, int icuTot, int icuAvail, int oxy);

    int getTotalBeds() const { return totalBeds; }
    int getAvailableBeds() const { return availableBeds; }
    int getTotalIcu() const { return totalIcu; }
    int getAvailableIcu() const { return availableIcu; }
    int getOxygenBeds() const { return oxygenBeds; }

    bool hasIcuCapacity() const { return availableIcu > 0; }
    bool hasGeneralCapacity() const { return availableBeds > 0; }

    std::string toJson() const;
};

#endif // M2_MODEL_BED_AVAILABILITY_H
