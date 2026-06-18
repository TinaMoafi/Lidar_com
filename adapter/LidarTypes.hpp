#pragma once
#include <vector>
#include <cstdint>

struct Ray {
    double   timestamp;
    uint32_t ray_id;
    float    x, y, z;
    float    intensity;
};

struct LidarFrame {
    uint32_t         ray_count;
    double           timestamp;   // from first ray
    std::vector<Ray> rays;
};