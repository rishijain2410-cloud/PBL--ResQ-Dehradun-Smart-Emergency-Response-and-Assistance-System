#pragma once
#include "../model/EmergencyContact.hpp"
#include "../model/EmergencyRequest.hpp"
#include "../model/EmergencyService.hpp"
#include <memory>
#include <cstddef>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

struct HigherEmergencyPriority {
    bool operator()(const EmergencyRequest& a, const EmergencyRequest& b) const {
        if (a.getPriority() != b.getPriority())
            return static_cast<int>(a.getPriority()) < static_cast<int>(b.getPriority());
        return a.getId() > b.getId();
    }
};

class EmergencyManager {
public:
    bool createEmergencyRequest(const EmergencyRequest& request);
    const EmergencyRequest* findRequestById(const std::string& id) const;
    std::size_t prioritizeEmergencyRequests();
    bool updateRequestStatus(const std::string& id, EmergencyStatus status);
    bool processNextEmergencyRequest(EmergencyRequest& out);
    std::vector<EmergencyRequest> getRequestHistory(const std::string& userId) const;
    std::vector<EmergencyRequest> getAllRequests() const;
    bool addEmergencyContact(const EmergencyContact& contact);
    std::vector<EmergencyContact> getEmergencyContacts(const std::string& userId) const;
    void addEmergencyService(std::shared_ptr<EmergencyService> service);
    std::vector<std::shared_ptr<const EmergencyService>> findAvailableServices(EmergencyServiceType type) const;
    std::vector<std::shared_ptr<const EmergencyService>> getEmergencyServices() const;
private:
    std::unordered_map<std::string, EmergencyRequest> itemMapById;
    std::queue<std::string> requestQueue;
    std::priority_queue<EmergencyRequest, std::vector<EmergencyRequest>, HigherEmergencyPriority> priorityQueue;
    std::unordered_map<std::string, EmergencyContact> contactsById_;
    std::vector<std::shared_ptr<EmergencyService>> services_;
};
