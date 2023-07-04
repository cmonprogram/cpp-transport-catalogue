#pragma once

#include <cmath>
namespace geo {
    const int earth_radius = 6371000;
    struct Coordinates {
        Coordinates() = default;
        Coordinates(double lat, double lng) : lat(lat), lng(lng) {}
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };
     double ComputeDistance(Coordinates from, Coordinates to);
}