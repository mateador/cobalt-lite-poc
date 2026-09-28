#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "frame.hpp"
#include "thread_safe_queue.hpp"
#include "protocol.hpp"
#include "network_io.hpp"

ThreadSafeQueue g_queue;
volatile bool g_running = true;

void signal_handler(int) { g_running = false; g_queue.shutdown(); }

// --- PRODUCER THREAD: Generates dummy frames ---
void frame_generator(int fps, int width, int height) {
    uint32_t frame_id = 0;
    size_t payload_size = width * height * 3; // Simulated RGB

    while (g_running) {
        // Create a dummy payload (in reality, this would be a captured screen)
        std::vector<uint8_t> payload(payload_size, static_cast<uint8_t>(frame_id % 256));
        auto frame = std::make_unique<Frame>(width, height, std::move(payload));
        frame->width = width;
        frame->height = height;

        g_queue.push(std::move(frame));
        frame_id++;

        // Throttle to target FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
    }
    std::cout << "[Sender] Frame generator stopped. Total frames: " << frame_id << "\n";
}

// --- CONSUMER THREAD: Sends frames over TCP ---
void network_sender(int client_fd) {
    uint32_t sent_count = 0;

    while (g_running) {
        std::unique_ptr<Frame> frame;
        if (!g_queue.wait_and_pop(frame)) break;

        // Build the network packet
        NetworkPacket packet;
        packet.header.frame_id = sent_count;
        packet.payload = std::move(frame->payload);

        // Serialize to wire format (handles endianness + CRC)
        auto wire_data = serialize_packet(packet);

        if (!send_all(client_fd, wire_data)) {
            std::cerr << "[Sender] Client disconnected.\n";
            break;
        }

        sent_count++;
        if (sent_count % 30 == 0) {
            std::cout << "[Sender] Sent " << sent_count << " frames ("
                      << wire_data.size() << " bytes/packet)\n";
        }
    }
    std::cout << "[Sender] Network sender stopped.\n";
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);

    int port = 9876;
    int fps = 30;
    int width = 320;   // Small for demo; real would be 1920
    int height = 240;

    if (argc > 1) port = std::stoi(argv[1]);

    std::cout << "=== Cobalt Lite Sender ===\n";
    std::cout << "Resolution: " << width << "x" << height << " @ " << fps << " FPS\n";
    std::cout << "Listening on port " << port << "...\n";

    // Create TCP server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed. Is port " << port << " in use?\n";
        return 1;
    }
    listen(server_fd, 1);
    std::cout << "[Sender] Waiting for receiver to connect...\n";

    int client_fd = accept(server_fd, nullptr, nullptr);
    std::cout << "[Sender] Receiver connected! Streaming...\n";

    // Launch threads
    std::thread producer(frame_generator, fps, width, height);
    std::thread consumer(network_sender, client_fd);

    producer.join();
    consumer.join();

    close(client_fd);
    close(server_fd);
    std::cout << "[Sender] Clean shutdown.\n";
    return 0;
}