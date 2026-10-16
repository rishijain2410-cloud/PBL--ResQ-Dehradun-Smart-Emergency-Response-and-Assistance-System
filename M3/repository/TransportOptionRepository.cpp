#include "repository/TransportOptionRepository.h"

using namespace std;

shared_ptr<TransportOption> InMemoryTransportOptionRepository::findByMode(TransportMode mode) const {
    for (const auto& item : itemList) {
        if (item->getMode() == mode) {
            return item;
        }
    }
    return nullptr;
}

vector<shared_ptr<TransportOption>> InMemoryTransportOptionRepository::findAll() const {
    return itemList;
}

bool InMemoryTransportOptionRepository::save(const shared_ptr<TransportOption>& option) {
    if (!option) {
        return false;
    }

    const string& id = option->getOptionId();
    if (uniqueIdSet.find(id) != uniqueIdSet.end()) {
        return false;
    }

    uniqueIdSet.insert(id);
    itemMapById.emplace(id, option);
    itemList.push_back(option);
    return true;
}

size_t InMemoryTransportOptionRepository::size() const {
    return itemList.size();
}

void InMemoryTransportOptionRepository::clear() {
    uniqueIdSet.clear();
    itemMapById.clear();
    itemList.clear();
}
