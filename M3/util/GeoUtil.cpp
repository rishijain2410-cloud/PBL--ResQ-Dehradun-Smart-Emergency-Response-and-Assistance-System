#include "util/GeoUtil.h"

#include <cmath>
#include <algorithm>

using namespace std;

namespace GeoUtil {

// WGS84 volumetric mean Earth radius in kilometers
static constexpr double EARTH_RADIUS_KM = 6371.0;
static constexpr double PI = 3.14159265358979323846;

static inline double degreesToRadians(double degrees) {
    return degrees * (PI / 180.0);
}

double haversineDistanceKm(double lat1, double lon1, double lat2, double lon2) {
    // Identity check: identical points produce zero distance immediately
    if (lat1 == lat2 && lon1 == lon2) {
        return 0.0;
    }

    double dLat = degreesToRadians(lat2 - lat1);
    double dLon = degreesToRadians(lon2 - lon1);

    double radLat1 = degreesToRadians(lat1);
    double radLat2 = degreesToRadians(lat2);

    double sinHalfDLat = sin(dLat / 2.0);
    double sinHalfDLon = sin(dLon / 2.0);

    double a = (sinHalfDLat * sinHalfDLat) +
               cos(radLat1) * cos(radLat2) * (sinHalfDLon * sinHalfDLon);

    // Guard against floating point rounding inaccuracies exceeding [0, 1]
    a = clamp(a, 0.0, 1.0);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}

} // namespace GeoUtil
