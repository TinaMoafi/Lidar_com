#include "UdpReceiver.hpp"
#include "LidarParser.hpp"

#include <vsomeip/vsomeip.hpp>

#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <set>
#include <chrono>

static constexpr vsomeip::service_t    SERVICE  = 0x1234;
static constexpr vsomeip::instance_t   INSTANCE = 0x0001;
static constexpr vsomeip::event_t      EVENT    = 0x8001;
static constexpr vsomeip::eventgroup_t GROUP    = 0x0001;
static constexpr int                   UDP_PORT = 25000;

static std::atomic<bool> g_running{true};

void on_signal(int) {
    g_running = false;
}

int main() {
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    auto app = vsomeip::runtime::get()->create_application("lidar_adapter");
    if (!app) {
        std::cerr << "[adapter] failed to create vsomeip application\n";
        return 1;
    }

    if (!app->init()) {
        std::cerr << "[adapter] failed to init vsomeip application\n";
        return 1;
    }

    std::mutex ready_mutex;
    std::condition_variable ready_cv;
    bool app_ready = false;

    app->register_state_handler([&](vsomeip::state_type_e state) {
        if (state == vsomeip::state_type_e::ST_REGISTERED) {
            std::set<vsomeip::eventgroup_t> groups{GROUP};

            app->offer_service(SERVICE, INSTANCE);
            app->offer_event(
                SERVICE,
                INSTANCE,
                EVENT,
                groups,
                vsomeip::event_type_e::ET_EVENT
            );

            {
                std::lock_guard<std::mutex> lock(ready_mutex);
                app_ready = true;
            }
            ready_cv.notify_one();

            std::cout << "[adapter] vsomeip registered, service/event offered\n";
        }
    });

    std::thread vsomeip_thread([&] {
        app->start();
    });

    {
        std::unique_lock<std::mutex> lock(ready_mutex);
        ready_cv.wait(lock, [&] { return app_ready || !g_running.load(); });
    }

    if (!app_ready) {
        std::cerr << "[adapter] vsomeip did not become ready\n";
        app->stop();
        if (vsomeip_thread.joinable()) vsomeip_thread.join();
        return 1;
    }

    LidarParser parser;
    UdpReceiver receiver(UDP_PORT);

    std::atomic<uint32_t> frame_count{0};

    std::thread udp_thread([&] {
        try {
            receiver.run([&](const std::string& packet) {
                auto frame = parser.parse(packet);
                if (!frame)
                    return;

                auto bytes = parser.serialize(*frame);

                auto payload = vsomeip::runtime::get()->create_payload();
                payload->set_data(bytes);

                app->notify(SERVICE, INSTANCE, EVENT, payload);

                const auto count = ++frame_count;
                std::cout << "[adapter] frame #" << count
                          << " rays=" << frame->rays.size()
                          << " t=" << frame->timestamp
                          << " payload_bytes=" << bytes.size()
                          << "\n";
            });
        } catch (const std::exception& e) {
            std::cerr << "[adapter] udp thread error: " << e.what() << "\n";
            g_running = false;
        }
    });

    while (g_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "[adapter] shutting down\n";

    receiver.stop();
    app->stop();

    if (udp_thread.joinable()) udp_thread.join();
    if (vsomeip_thread.joinable()) vsomeip_thread.join();

    return 0;
}