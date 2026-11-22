#ifndef M3_UTIL_GEO_UTIL_H
#define M3_UTIL_GEO_UTIL_H

class GeoUtil {
public:
    static constexpr double EARTH_RADIUS_KM = 6371.0;

    // Calculates Great-Circle distance between two GPS points in kilometers using Haversine formula
    static double haversineDistanceKm(double lat1, double lon1, double lat2, double lon2);

    // Converts degrees to radians
    static double toRadians(double degrees);

    // Converts radians to degrees
    static double toDegrees(double radians);
};

#endif // M3_UTIL_GEO_UTIL_H
