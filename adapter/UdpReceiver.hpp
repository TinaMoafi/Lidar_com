#pragma once
#include <string>
#include <functional>

// Calls `on_packet` with the raw string contents of each received UDP packet.
// Blocking — run in its own thread.
class UdpReceiver {
public:
    explicit UdpReceiver(int port, size_t buf_size = 65535);
    ~UdpReceiver();

    using PacketCallback = std::function<void(const std::string&)>;
    void run(const PacketCallback& on_packet);  // blocks until stop()
    void stop();

private:
    int    port_;
    size_t buf_size_;
    int    sockfd_ = -1;
    bool   running_ = false;
};