#ifndef M3_SERVICE_TRANSPORT_ESTIMATOR_H
#define M3_SERVICE_TRANSPORT_ESTIMATOR_H

#include "../model/TransportOptions.h"
#include "../dto/RouteResponse.h"
#include <vector>
#include <memory>

class TransportEstimator {
private:
    std::vector<std::shared_ptr<TransportOption>> options;

public:
    TransportEstimator();

    void estimateAll(RouteResponse& route) const;
    const std::vector<std::shared_ptr<TransportOption>>& getOptions() const { return options; }
};

#endif // M3_SERVICE_TRANSPORT_ESTIMATOR_H
