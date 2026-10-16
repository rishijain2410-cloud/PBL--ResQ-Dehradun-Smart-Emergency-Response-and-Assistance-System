#pragma once

#include "model/TransportOption.h"
#include "model/TransportMode.h"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

class ITransportOptionRepository {
public:
    virtual ~ITransportOptionRepository() = default;

    virtual std::shared_ptr<TransportOption> findByMode(TransportMode mode) const = 0;
    virtual std::vector<std::shared_ptr<TransportOption>> findAll() const = 0;
    virtual bool save(const std::shared_ptr<TransportOption>& option) = 0;
};

class InMemoryTransportOptionRepository : public ITransportOptionRepository {
private:
    std::vector<std::shared_ptr<TransportOption>> itemList;
    std::unordered_map<std::string, std::shared_ptr<TransportOption>> itemMapById;
    std::unordered_set<std::string> uniqueIdSet;

public:
    InMemoryTransportOptionRepository() = default;
    ~InMemoryTransportOptionRepository() override = default;

    std::shared_ptr<TransportOption> findByMode(TransportMode mode) const override;
    std::vector<std::shared_ptr<TransportOption>> findAll() const override;
    bool save(const std::shared_ptr<TransportOption>& option) override;

    std::size_t size() const;
    void clear();
};
