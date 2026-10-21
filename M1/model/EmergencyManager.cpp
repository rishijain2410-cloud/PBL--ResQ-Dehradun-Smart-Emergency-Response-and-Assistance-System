#include "EmergencyManager.hpp"
#include <cctype>
#include <algorithm>

namespace {
bool validPhone(const std::string& phone) {
    int digits = 0;
    for (unsigned char c : phone) {
        if (std::isdigit(c)) ++digits;
        else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') return false;
    }
    return digits >= 7 && digits <= 15;
}
bool validTransition(EmergencyStatus from, EmergencyStatus to) {
    if (to == EmergencyStatus::CANCELLED)
        return from != EmergencyStatus::COMPLETED && from != EmergencyStatus::CANCELLED;
    switch (from) {
        case EmergencyStatus::PENDING: return to == EmergencyStatus::ASSIGNED;
        case EmergencyStatus::ASSIGNED: return to == EmergencyStatus::ACCEPTED;
        case EmergencyStatus::ACCEPTED: return to == EmergencyStatus::EN_ROUTE;
        case EmergencyStatus::EN_ROUTE: return to == EmergencyStatus::COMPLETED;
        case EmergencyStatus::COMPLETED:
        case EmergencyStatus::CANCELLED: return false;
    }
    return false;
}
}

bool EmergencyManager::createEmergencyRequest(const EmergencyRequest& request) {
    if (request.getId().empty() || request.getUserId().empty() ||
        request.getLocation().locationId.empty() || request.getLocation().address.empty() ||
        itemMapById.count(request.getId())) return false;
    itemMapById.emplace(request.getId(), request);
    requestQueue.push(request.getId());
    return true;
}

const EmergencyRequest* EmergencyManager::findRequestById(const std::string& id) const {
    auto it = itemMapById.find(id);
    return it == itemMapById.end() ? nullptr : &it->second;
}

std::size_t EmergencyManager::prioritizeEmergencyRequests() {
    std::size_t moved = 0;
    while (!requestQueue.empty()) {
        const std::string id = requestQueue.front();
        requestQueue.pop();
        const auto it = itemMapById.find(id);
        if (it != itemMapById.end() && it->second.getStatus() == EmergencyStatus::PENDING) {
            priorityQueue.push(it->second);
            ++moved;
        }
    }
    return moved;
}

bool EmergencyManager::updateRequestStatus(const std::string& id, EmergencyStatus status) {
    auto it = itemMapById.find(id);
    if (it == itemMapById.end() || !validTransition(it->second.getStatus(), status)) return false;
    it->second.setStatus(status);
    return true;
}

bool EmergencyManager::processNextEmergencyRequest(EmergencyRequest& out) {
    if (priorityQueue.empty()) prioritizeEmergencyRequests();
    while (!priorityQueue.empty()) {
        EmergencyRequest next = priorityQueue.top();
        priorityQueue.pop();
        const auto it = itemMapById.find(next.getId());
        if (it == itemMapById.end() || it->second.getStatus() != EmergencyStatus::PENDING) continue;
        it->second.setStatus(EmergencyStatus::ASSIGNED);
        out = it->second;
        return true;
    }
    return false;
}

std::vector<EmergencyRequest> EmergencyManager::getRequestHistory(const std::string& userId) const {
    std::vector<EmergencyRequest> history;
    for (const auto& entry : itemMapById)
        if (entry.second.getUserId() == userId) history.push_back(entry.second);
    std::sort(history.begin(), history.end(), [](const EmergencyRequest& a, const EmergencyRequest& b) {
        return a.getId() < b.getId();
    });
    return history;
}

std::vector<EmergencyRequest> EmergencyManager::getAllRequests() const {
    std::vector<EmergencyRequest> history;
    for (const auto& entry : itemMapById) history.push_back(entry.second);
    std::sort(history.begin(), history.end(), [](const EmergencyRequest& a, const EmergencyRequest& b) {
        return a.getId() > b.getId();
    });
    return history;
}

bool EmergencyManager::addEmergencyContact(const EmergencyContact& contact) {
    if (contact.getId().empty() || contact.getUserId().empty() || contact.getName().empty() ||
        contact.getRelationship().empty() || !validPhone(contact.getPhone()) ||
        contactsById_.count(contact.getId())) return false;
    contactsById_.emplace(contact.getId(), contact);
    return true;
}

std::vector<EmergencyContact> EmergencyManager::getEmergencyContacts(const std::string& userId) const {
    std::vector<EmergencyContact> result;
    for (const auto& entry : contactsById_)
        if (entry.second.getUserId() == userId) result.push_back(entry.second);
    return result;
}

void EmergencyManager::addEmergencyService(std::shared_ptr<EmergencyService> service) {
    if (service) services_.push_back(std::move(service));
}

std::vector<std::shared_ptr<const EmergencyService>>
EmergencyManager::findAvailableServices(EmergencyServiceType type) const {
    std::vector<std::shared_ptr<const EmergencyService>> result;
    for (const auto& service : services_)
        if (service->isAvailable() && service->getType() == type) result.push_back(service);
    return result;
}

std::vector<std::shared_ptr<const EmergencyService>> EmergencyManager::getEmergencyServices() const {
    std::vector<std::shared_ptr<const EmergencyService>> result;
    for (const auto& service : services_) result.push_back(service);
    return result;
}
