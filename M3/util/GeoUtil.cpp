#include "GeoUtil.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double GeoUtil::toRadians(double degrees) {
    return degrees * (M_PI / 180.0);
}

double GeoUtil::toDegrees(double radians) {
    return radians * (180.0 / M_PI);
}

double GeoUtil::haversineDistanceKm(double lat1, double lon1, double lat2, double lon2) {
    // If coordinates are identical, return 0.0 directly
    if (lat1 == lat2 && lon1 == lon2) {
        return 0.0;
    }

    double phi1 = toRadians(lat1);
    double phi2 = toRadians(lat2);
    double deltaPhi = toRadians(lat2 - lat1);
    double deltaLambda = toRadians(lon2 - lon1);

    double sinDeltaPhi2 = std::sin(deltaPhi / 2.0);
    double sinDeltaLambda2 = std::sin(deltaLambda / 2.0);

    double a = (sinDeltaPhi2 * sinDeltaPhi2) +
               std::cos(phi1) * std::cos(phi2) * (sinDeltaLambda2 * sinDeltaLambda2);

    // Prevent floating-point inaccuracy exceeding 1.0
    if (a > 1.0) a = 1.0;
    if (a < 0.0) a = 0.0;

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}
