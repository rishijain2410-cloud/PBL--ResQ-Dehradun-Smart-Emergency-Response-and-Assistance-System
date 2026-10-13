#pragma once

namespace GeoUtil {

/**
 * Calculates the great-circle distance between two geographic coordinates
 * on Earth using the Haversine formula.
 *
 * Mathematical Formula:
 *   dLat = (lat2 - lat1) * (PI / 180)
 *   dLon = (lon2 - lon1) * (PI / 180)
 *   a = sin^2(dLat / 2) + cos(lat1 * PI / 180) * cos(lat2 * PI / 180) * sin^2(dLon / 2)
 *   c = 2 * atan2(sqrt(a), sqrt(1 - a))
 *   d = R * c  (where Earth mean radius R = 6371.0 km)
 *
 * Computational Complexity:
 * - Time Complexity: O(1) - Evaluates a fixed sequence of basic trigonometric and algebraic operations.
 * - Space Complexity: O(1) - Constant auxiliary storage.
 *
 * @param lat1 Latitude of point 1 in degrees [-90.0, 90.0]
 * @param lon1 Longitude of point 1 in degrees [-180.0, 180.0]
 * @param lat2 Latitude of point 2 in degrees [-90.0, 90.0]
 * @param lon2 Longitude of point 2 in degrees [-180.0, 180.0]
 * @return Distance between the two points in kilometers.
 */
double haversineDistanceKm(double lat1, double lon1, double lat2, double lon2);

} // namespace GeoUtil
