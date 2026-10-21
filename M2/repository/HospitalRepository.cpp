#include "HospitalRepository.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

bool HospitalRepository::loadFromCSV(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
        return false;

    std::string line;

    // Read and skip the CSV header.
    if (!std::getline(file, line))
        return false;

    std::unordered_map<std::string, Hospital> loadedHospitals;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::vector<std::string> fields;
        std::string field;

        while (std::getline(ss, field, ','))
            fields.push_back(field);

        // Expected columns:
        // id,name,address,contact,rating,locationId,generalBeds,icuBeds
        if (fields.size() != 8)
            return false;

        try
        {
            const std::string& id = fields[0];

            if (id.empty() || loadedHospitals.count(id))
                return false;

            const std::string& name = fields[1];
            const std::string& address = fields[2];
            const std::string& contact = fields[3];

            std::size_t ratingPosition = 0;
            double rating = std::stod(fields[4], &ratingPosition);

            if (ratingPosition != fields[4].size() ||
                rating < 0.0 || rating > 5.0)
                return false;

            const std::string& locationId = fields[5];

            std::size_t generalPosition = 0;
            int generalBeds = std::stoi(fields[6], &generalPosition);

            if (generalPosition != fields[6].size() || generalBeds < 0)
                return false;

            std::size_t icuPosition = 0;
            int icuBeds = std::stoi(fields[7], &icuPosition);

            if (icuPosition != fields[7].size() || icuBeds < 0)
                return false;

            Hospital hospital(
                id,
                name,
                address,
                contact,
                rating,
                locationId,
                BedAvailability(generalBeds, icuBeds)
            );

            loadedHospitals.emplace(id, hospital);
        }
        catch (...)
        {
            return false;
        }
    }

    // Replace existing in-memory data only after successful loading.
    itemMapById = std::move(loadedHospitals);

    return true;
}

bool HospitalRepository::saveToCSV(const std::string& filename) const
{
    std::ofstream file(filename, std::ios::trunc);

    if (!file.is_open())
        return false;

    file << "id,name,address,contact,rating,locationId,"
            "generalBeds,icuBeds\n";

    std::vector<Hospital> hospitals = findAll();

    for (const Hospital& hospital : hospitals)
    {
        file << hospital.getId() << ','
             << hospital.getName() << ','
             << hospital.getAddress() << ','
             << hospital.getContact() << ','
             << std::fixed << std::setprecision(1)
             << hospital.getRating() << ','
             << hospital.getLocationId() << ','
             << hospital.getBedAvailability().getGeneralBeds() << ','
             << hospital.getBedAvailability().getIcuBeds()
             << '\n';

        if (!file)
            return false;
    }

    file.close();

    return !file.fail();
}

bool HospitalRepository::add(const Hospital& hospital)
{
    if (hospital.getId().empty() ||
        itemMapById.count(hospital.getId()))
        return false;

    itemMapById.emplace(hospital.getId(), hospital);

    return true;
}

bool HospitalRepository::update(const Hospital& hospital)
{
    auto it = itemMapById.find(hospital.getId());

    if (it == itemMapById.end())
        return false;

    it->second = hospital;

    return true;
}

const Hospital* HospitalRepository::findHospitalById(
    const std::string& id
) const
{
    auto it = itemMapById.find(id);

    return it == itemMapById.end() ? nullptr : &it->second;
}

std::vector<Hospital> HospitalRepository::findAll() const
{
    std::vector<Hospital> itemList;

    itemList.reserve(itemMapById.size());

    for (const auto& entry : itemMapById)
        itemList.push_back(entry.second);

    std::sort(
        itemList.begin(),
        itemList.end(),
        [](const Hospital& a, const Hospital& b)
        {
            return a.getId() < b.getId();
        }
    );

    return itemList;
}