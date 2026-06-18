#include "UdpReceiver.hpp"
#include "LidarParser.hpp"
#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>

// ── SOME/IP identifiers — must match vsomeip_adapter.json ────────────────────
static constexpr vsomeip::service_t    SERVICE = 0x1234;
static constexpr vsomeip::instance_t   INSTANCE = 0x0001;
static constexpr vsomeip::event_t      EVENT = 0x8001;
static constexpr vsomeip::eventgroup_t GROUP = EVENT;
static constexpr int                   UDP_PORT = 25000;

static std::atomic<bool> g_running{ true };

void on_signal(int) { g_running = false; }

int main() {
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    // ── vsomeip setup ─────────────────────────────────────────────────────────
    auto app = vsomeip::runtime::get()->create_application("lidar_adapter");
    app->init();
    app->offer_service(SERVICE, INSTANCE);
    std::set<vsomeip::eventgroup_t> groups{ GROUP };
    app->offer_event(SERVICE, INSTANCE, EVENT, groups,
        vsomeip::event_type_e::ET_FIELD);

    std::thread vsomeip_thread([&] { app->start(); });

    // ── parser + receiver ─────────────────────────────────────────────────────
    LidarParser  parser;
    UdpReceiver  receiver(UDP_PORT);

    uint32_t frame_count = 0;

    std::thread udp_thread([&] {
        receiver.run([&](const std::string& packet) {

            auto frame = parser.parse(packet);
            if (!frame) return;

            auto bytes = parser.serialize(*frame);

            auto payload = vsomeip::runtime::get()->create_payload();
            payload->set_data(bytes);
            app->notify(SERVICE, INSTANCE, EVENT, payload);

            std::cout << "[adapter] frame #" << ++frame_count
                << "  rays=" << frame->ray_count
                << "  t=" << frame->timestamp << "\n";
            });
        });

    // ── wait for Ctrl-C ───────────────────────────────────────────────────────
    while (g_running) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "[adapter] shutting down\n";
    receiver.stop();
    app->stop();
    udp_thread.join();
    vsomeip_thread.join();
    return 0;
}