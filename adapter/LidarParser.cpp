#include "LidarParser.hpp"
#include <sstream>
#include <iostream>
#include <cstring>

namespace {
    constexpr uint32_t LIDAR_MAGIC = 0x4C444652; // "LDFR"
    constexpr uint16_t LIDAR_VERSION = 1;

    void append_u16_le(std::vector<uint8_t>& buf, uint16_t value) {
        buf.push_back(static_cast<uint8_t>(value & 0xFF));
        buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    }

    void append_u32_le(std::vector<uint8_t>& buf, uint32_t value) {
        buf.push_back(static_cast<uint8_t>(value & 0xFF));
        buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buf.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        buf.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    }

    void append_u64_le(std::vector<uint8_t>& buf, uint64_t value) {
        for (int i = 0; i < 8; ++i)
            buf.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));
    }

    void append_f32_le(std::vector<uint8_t>& buf, float value) {
        static_assert(sizeof(float) == sizeof(uint32_t));
        uint32_t raw = 0;
        std::memcpy(&raw, &value, sizeof(raw));
        append_u32_le(buf, raw);
    }

    void append_f64_le(std::vector<uint8_t>& buf, double value) {
        static_assert(sizeof(double) == sizeof(uint64_t));
        uint64_t raw = 0;
        std::memcpy(&raw, &value, sizeof(raw));
        append_u64_le(buf, raw);
    }
}

std::optional<LidarFrame> LidarParser::parse(const std::string& packet) {
    LidarFrame frame;
    std::istringstream stream(packet);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        auto ray = parse_ray(line);
        if (!ray) continue;

        frame.rays.push_back(*ray);
    }

    if (frame.rays.empty())
        return std::nullopt;

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
        Ray r{};
        r.timestamp = std::stod(fields[0]);
        r.ray_id = static_cast<uint32_t>(std::stoul(fields[1]));
        r.x = std::stof(fields[2]);
        r.y = std::stof(fields[3]);
        r.z = std::stof(fields[4]);
        r.intensity = std::stof(fields[5]);
        return r;
    } catch (const std::exception& e) {
        std::cerr << "[parser] parse error on line \"" << line
                  << "\": " << e.what() << "\n";
        return std::nullopt;
    }
}

std::vector<uint8_t> LidarParser::serialize(const LidarFrame& frame) {
    const uint32_t ray_count = static_cast<uint32_t>(frame.rays.size());

    const size_t header_size =
        sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) +
        sizeof(uint32_t) + sizeof(uint64_t);

    const size_t ray_size =
        sizeof(uint64_t) + sizeof(uint32_t) + 4 * sizeof(uint32_t);

    std::vector<uint8_t> buf;
    buf.reserve(header_size + ray_count * ray_size);

    append_u32_le(buf, LIDAR_MAGIC);
    append_u16_le(buf, LIDAR_VERSION);
    append_u16_le(buf, 0);
    append_u32_le(buf, ray_count);
    append_f64_le(buf, frame.timestamp);

    for (const auto& r : frame.rays) {
        append_f64_le(buf, r.timestamp);
        append_u32_le(buf, r.ray_id);
        append_f32_le(buf, r.x);
        append_f32_le(buf, r.y);
        append_f32_le(buf, r.z);
        append_f32_le(buf, r.intensity);
    }

    return buf;
}