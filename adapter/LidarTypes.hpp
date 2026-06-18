#pragma once
#include <vector>
#include <cstdint>

struct Ray {
    double   timestamp;
    uint32_t ray_id;
    float    x;
    float    y;
    float    z;
    float    intensity;
};

struct LidarFrame {
    double           timestamp = 0.0;
    std::vector<Ray> rays;
};