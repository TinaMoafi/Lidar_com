#pragma once
#include "LidarTypes.hpp"
#include <string>
#include <optional>
#include <vector>
#include <cstdint>

class LidarParser {
public:
    std::optional<LidarFrame> parse(const std::string& packet);
    std::vector<uint8_t> serialize(const LidarFrame& frame);

private:
    std::optional<Ray> parse_ray(const std::string& line);
};