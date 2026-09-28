#include <iomanip>
#include <iostream>
#include <chrono>
#include <cstring>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "protocol.hpp"
#include "network_io.hpp"

volatile bool g_running = true;
void signal_handler(int) { g_running = false; }

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);

    std::string server_ip = "127.0.0.1";
    int port = 9876;
    if (argc > 1) server_ip = argv[1];
    if (argc > 2) port = std::stoi(argv[2]);

    std::cout << "=== Cobalt Lite Receiver ===\n";
    std::cout << "Connecting to " << server_ip << ":" << port << "...\n";

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &addr.sin_addr);

    if (connect(sock_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection failed. Is the sender running?\n";
        return 1;
    }
    std::cout << "[Receiver] Connected! Receiving frames...\n";

    uint32_t received = 0;
    uint32_t corrupted = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (g_running) {
        // 1. Read the fixed-size header first
        std::vector<uint8_t> header_buf;
        if (!read_exact(sock_fd, header_buf, sizeof(PacketHeader))) break;

        // Parse header to learn payload size (handle endianness)
        PacketHeader net_hdr;
        std::memcpy(&net_hdr, header_buf.data(), sizeof(PacketHeader));
        uint32_t payload_size = ntohl(net_hdr.payload_size);
        uint32_t magic = ntohl(net_hdr.magic);

        if (magic != COBALT_MAGIC) {
            std::cerr << "[Receiver] Lost sync! Invalid magic number.\n";
            break;
        }

        // 2. Read the variable-size payload
        std::vector<uint8_t> payload_buf;
        if (!read_exact(sock_fd, payload_buf, payload_size)) break;

        // 3. Reassemble full packet for validation
        std::vector<uint8_t> full_packet;
        full_packet.insert(full_packet.end(), header_buf.begin(), header_buf.end());
        full_packet.insert(full_packet.end(), payload_buf.begin(), payload_buf.end());

        NetworkPacket packet;
        if (deserialize_packet(full_packet, packet)) {
            received++;
        } else {
            corrupted++;
        }

        // Print stats every 30 frames
        if (received % 30 == 0 && received > 0) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - start_time).count();
            double fps = received / elapsed;
            std::cout << "[Receiver] Frame #" << packet.header.frame_id
                      << " | " << payload_size << " bytes"
                      << " | CRC: OK"
                      << " | Avg FPS: " << std::fixed << std::setprecision(1) << fps
                      << " | Corrupted: " << corrupted << "\n";
        }
    }

    close(sock_fd);
    std::cout << "\n[Receiver] === Session Summary ===\n";
    std::cout << "  Frames received: " << received << "\n";
    std::cout << "  Corrupted:       " << corrupted << "\n";
    std::cout << "  Clean shutdown.\n";
    return 0;
}