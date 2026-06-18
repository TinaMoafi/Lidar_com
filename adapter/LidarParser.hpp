#pragma once
#include "LidarTypes.hpp"
#include <string>
#include <optional>

class LidarParser {
public:
    // Parse a full UDP packet (newline-separated rays) into a LidarFrame.
    // Returns std::nullopt if the packet is empty or completely malformed.
    std::optional<LidarFrame> parse(const std::string& packet);

    // Serialize a LidarFrame into a flat byte buffer for SOME/IP payload.
    std::vector<uint8_t> serialize(const LidarFrame& frame);

private:
    std::optional<Ray> parse_ray(const std::string& line);
};