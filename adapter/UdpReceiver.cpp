#include "UdpReceiver.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

UdpReceiver::UdpReceiver(int port, std::size_t buf_size)
    : port_(port), buf_size_(buf_size) {}

UdpReceiver::~UdpReceiver() {
    stop();
}

void UdpReceiver::run(const PacketCallback& on_packet) {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0)
        throw std::runtime_error("socket() failed");

    int opt = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(sockfd_);
        sockfd_ = -1;
        throw std::runtime_error("bind() failed on port " + std::to_string(port_));
    }

    std::cout << "[udp] listening on :" << port_ << "\n";

    std::vector<char> buf(buf_size_);
    running_.store(true);

    while (running_.load()) {
        ssize_t len = recvfrom(sockfd_, buf.data(), buf_size_ - 1, 0, nullptr, nullptr);

        if (len < 0) {
            if (!running_.load())
                break;
            if (errno == EINTR)
                continue;

            std::cerr << "[udp] recvfrom error: " << std::strerror(errno) << "\n";
            continue;
        }

        if (len == 0)
            continue;

        buf[static_cast<std::size_t>(len)] = '\0';
        on_packet(std::string(buf.data(), static_cast<std::size_t>(len)));
    }

    std::cout << "[udp] receiver stopped\n";
}

void UdpReceiver::stop() {
    const bool was_running = running_.exchange(false);
    if (sockfd_ >= 0) {
        shutdown(sockfd_, SHUT_RDWR);
        close(sockfd_);
        sockfd_ = -1;
    }
    if (was_running) {
        std::cout << "[udp] stop requested\n";
    }
}