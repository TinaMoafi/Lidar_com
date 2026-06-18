#include "LidarParser.hpp"
#include <sstream>
#include <iostream>
#include <cstring>

std::optional<LidarFrame> LidarParser::parse(const std::string& packet) {
    LidarFrame frame;
    std::istringstream stream(packet);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        auto ray = parse_ray(line);
        if (!ray) continue;          // log and skip malformed lines

        frame.rays.push_back(*ray);
    }

    if (frame.rays.empty()) return std::nullopt;

    // Use first ray's timestamp as the frame timestamp
    frame.ray_count = static_cast<uint32_t>(frame.rays.size());
    frame.timestamp = frame.rays.front().timestamp;

    return frame;
}

std::optional<Ray> LidarParser::parse_ray(const std::string& line) {
    std::istringstream ls(line);
    std::string tok;
    std::vector<std::string> fields;

    while (std::getline(ls, tok, ';'))
        fields.push_back(tok);

    if (fields.size() != 6) {
        std::cerr << "[parser] malformed ray (expected 6 fields, got "
            << fields.size() << "): " << line << "\n";
        return std::nullopt;
    }

    try {
        Ray r;
        r.timestamp = std::stod(fields[0]);
        r.ray_id = static_cast<uint32_t>(std::stoul(fields[1]));
        r.x = std::stof(fields[2]);
        r.y = std::stof(fields[3]);
        r.z = std::stof(fields[4]);
        r.intensity = std::stof(fields[5]);
        return r;
    }
    catch (const std::exception& e) {
        std::cerr << "[parser] parse error on line \"" << line
            << "\": " << e.what() << "\n";
        return std::nullopt;
    }
}

// Wire layout per frame:
// [ uint32 ray_count ]
// [ double timestamp ]
// per ray: [ double ts | uint32 ray_id | float x | float y | float z | float intensity ]
std::vector<uint8_t> LidarParser::serialize(const LidarFrame& frame) {
    // Pre-calculate exact size to avoid reallocations
    // header: 4 (ray_count) + 8 (timestamp)
    // per ray: 8 + 4 + 4 + 4 + 4 + 4 = 28 bytes
    const size_t ray_size = sizeof(double) + sizeof(uint32_t) + 4 * sizeof(float);
    const size_t total = sizeof(uint32_t) + sizeof(double) + frame.ray_count * ray_size;

    std::vector<uint8_t> buf;
    buf.reserve(total);

    auto push = [&](const void* src, size_t n) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(src);
        buf.insert(buf.end(), p, p + n);
        };

    push(&frame.ray_count, sizeof(frame.ray_count));
    push(&frame.timestamp, sizeof(frame.timestamp));

    for (const auto& r : frame.rays) {
        push(&r.timestamp, sizeof(r.timestamp));
        push(&r.ray_id, sizeof(r.ray_id));
        push(&r.x, sizeof(r.x));
        push(&r.y, sizeof(r.y));
        push(&r.z, sizeof(r.z));
        push(&r.intensity, sizeof(r.intensity));
    }

    return buf;
}