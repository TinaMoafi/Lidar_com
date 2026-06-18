#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <cstddef>

class UdpReceiver {
public:
    explicit UdpReceiver(int port, std::size_t buf_size = 65535);
    ~UdpReceiver();

    using PacketCallback = std::function<void(const std::string&)>;
    void run(const PacketCallback& on_packet);
    void stop();

private:
    int port_;
    std::size_t buf_size_;
    int sockfd_ = -1;
    std::atomic<bool> running_{false};
};