#include "UdpReceiver.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

UdpReceiver::UdpReceiver(int port, size_t buf_size)
    : port_(port), buf_size_(buf_size) {}

UdpReceiver::~UdpReceiver() {
    stop();
}

void UdpReceiver::run(const PacketCallback& on_packet) {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0)
        throw std::runtime_error("socket() failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed on port " + std::to_string(port_));

    std::cout << "[udp] listening on :" << port_ << "\n";

    std::vector<char> buf(buf_size_);
    running_ = true;

    while (running_) {
        ssize_t len = recvfrom(sockfd_, buf.data(), buf_size_ - 1, 0, nullptr, nullptr);
        if (len <= 0) continue;
        buf[len] = '\0';
        on_packet(std::string(buf.data(), len));
    }
}

void UdpReceiver::stop() {
    running_ = false;
    if (sockfd_ >= 0) {
        close(sockfd_);
        sockfd_ = -1;
    }
}