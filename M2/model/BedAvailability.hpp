#pragma once

class BedAvailability {
public:
    BedAvailability(int generalBeds = 0, int icuBeds = 0) : generalBeds_(generalBeds), icuBeds_(icuBeds) {}
    int getGeneralBeds() const { return generalBeds_; }
    int getIcuBeds() const { return icuBeds_; }
    bool hasAnyBed() const { return generalBeds_ > 0 || icuBeds_ > 0; }
    void setGeneralBeds(int count) { if (count >= 0) generalBeds_ = count; }
    void setIcuBeds(int count) { if (count >= 0) icuBeds_ = count; }
private:
    int generalBeds_;
    int icuBeds_;
};
