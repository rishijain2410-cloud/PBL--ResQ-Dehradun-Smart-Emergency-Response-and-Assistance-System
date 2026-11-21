#include "HospitalRepository.h"

HospitalRepository::HospitalRepository() {
    seedDehradunHospitals();
}

bool HospitalRepository::save(const Hospital& hospital) {
    std::lock_guard<std::mutex> lock(repoMutex);
    storage[hospital.getId()] = hospital;
    return true;
}

std::optional<Hospital> HospitalRepository::findById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(repoMutex);
    auto it = storage.find(id);
    if (it != storage.end()) return it->second;
    return std::nullopt;
}

std::vector<Hospital> HospitalRepository::findAll() const {
    std::lock_guard<std::mutex> lock(repoMutex);
    std::vector<Hospital> list;
    for (const auto& kv : storage) {
        list.push_back(kv.second);
    }
    return list;
}

size_t HospitalRepository::count() const {
    std::lock_guard<std::mutex> lock(repoMutex);
    return storage.size();
}

void HospitalRepository::seedDehradunHospitals() {
    // 1. Max Super Speciality Hospital
    Hospital maxH("LOC-004", "Max Super Speciality Hospital",
                 "Mussoorie Diversion Road, Johri, Malsi, Dehradun - 248001",
                 "+91 135 716 2222", "24/7 Available",
                 BedAvailability(220, 48, 28, 6, 45), 30.3812, 78.0772, 4.8);
    maxH.addSpecialty("Trauma & Emergency");
    maxH.addSpecialty("Cardiology");
    maxH.addSpecialty("Neurology");
    maxH.addSpecialty("Oncology");
    maxH.addSpecialty("Orthopedics");
    Department maxCardio("Cardiology", "Dr. Arvind Sharma");
    maxCardio.addDoctor(Doctor("DOC-M01", "Dr. Arvind Sharma", "Cardiology", "Interventional Cardiologist"));
    maxH.addDepartment(maxCardio);
    save(maxH);

    // 2. Shri Mahant Indiresh Hospital
    Hospital smih("LOC-005", "Shri Mahant Indiresh Hospital (SMIH)",
                  "Patel Nagar, Dehradun, Uttarakhand - 248001",
                  "+91 135 667 2600", "24/7 Trauma Center",
                  BedAvailability(1500, 310, 120, 18, 250), 30.3124, 78.0289, 4.6);
    smih.addSpecialty("Trauma & Emergency");
    smih.addSpecialty("General Care");
    smih.addSpecialty("Nephrology");
    smih.addSpecialty("Cardiology");
    save(smih);

    // 3. Synergy Institute of Medical Sciences
    Hospital syn("LOC-006", "Synergy Institute of Medical Sciences",
                 "Ballupur Chowk, Dehradun, Uttarakhand - 248001",
                 "+91 135 253 1111", "24/7 Critical Care",
                 BedAvailability(180, 32, 45, 9, 60), 30.3345, 78.0212, 4.7);
    syn.addSpecialty("Neurology");
    syn.addSpecialty("Cardiology");
    syn.addSpecialty("Trauma & Emergency");
    syn.addSpecialty("Critical Care");
    save(syn);

    // 4. Graphic Era Institute of Medical Sciences (GEIMS)
    Hospital geims("LOC-007", "Graphic Era Institute of Medical Sciences (GEIMS)",
                   "16th Milestone, Chakrata Road, Dhoolkot, Dehradun - 248008",
                   "+91 180 088 97351", "24/7 Emergency",
                   BedAvailability(500, 140, 80, 14, 120), 30.3521, 77.9654, 4.9);
    geims.addSpecialty("Trauma & Emergency");
    geims.addSpecialty("General Care");
    geims.addSpecialty("Orthopedics");
    geims.addSpecialty("Paediatrics");
    save(geims);

    // 5. Doon Government Hospital
    Hospital doon("LOC-011", "Doon Government Hospital",
                  "New Road, Race Course, Dehradun, Uttarakhand - 248001",
                  "+91 135 265 9227", "24/7 Govt Emergency",
                  BedAvailability(350, 65, 30, 4, 90), 30.3185, 78.0412, 4.3);
    doon.addSpecialty("Trauma & Emergency");
    doon.addSpecialty("General Care");
    save(doon);

    // 6. CMI Hospital
    Hospital cmi("LOC-009", "CMI Hospital",
                 "54 Haridwar Road, Dharampur, Dehradun, Uttarakhand - 248001",
                 "+91 135 262 2333", "24/7 Emergency Wing",
                 BedAvailability(100, 22, 20, 3, 40), 30.3142, 78.0511, 4.4);
    cmi.addSpecialty("General Care");
    cmi.addSpecialty("Gynaecology");
    cmi.addSpecialty("Paediatrics");
    save(cmi);
}
