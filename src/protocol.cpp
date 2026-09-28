#include "protocol.hpp"
#include <arpa/inet.h> // For htonl, ntohl (POSIX/Linux/Android)
#include <cstring>

// A standard, lightweight CRC32 implementation (IEEE polynomial)
uint32_t calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

std::vector<uint8_t> serialize_packet(const NetworkPacket& packet) {
    std::vector<uint8_t> buffer(sizeof(PacketHeader) + packet.payload.size());
    
    // 1. Populate header in HOST byte order
    PacketHeader host_header;
    host_header.magic = COBALT_MAGIC;
    host_header.frame_id = packet.header.frame_id;
    host_header.payload_size = static_cast<uint32_t>(packet.payload.size());
    host_header.checksum = calculate_crc32(packet.payload.data(), packet.payload.size());

    // 2. Convert multi-byte fields to NETWORK byte order (Big-Endian)
    PacketHeader net_header;
    net_header.magic = htonl(host_header.magic);
    net_header.frame_id = htonl(host_header.frame_id);
    net_header.payload_size = htonl(host_header.payload_size);
    net_header.checksum = htonl(host_header.checksum);

    // 3. Copy to buffer
    std::memcpy(buffer.data(), &net_header, sizeof(PacketHeader));
    if (!packet.payload.empty()) {
        std::memcpy(buffer.data() + sizeof(PacketHeader), packet.payload.data(), packet.payload.size());
    }
    return buffer;
}

bool deserialize_packet(const std::vector<uint8_t>& raw_data, NetworkPacket& out_packet) {
    if (raw_data.size() < sizeof(PacketHeader)) return false;

    // 1. Read header from NETWORK byte order
    PacketHeader net_header;
    std::memcpy(&net_header, raw_data.data(), sizeof(PacketHeader));

    // 2. Convert back to HOST byte order
    PacketHeader host_header;
    host_header.magic = ntohl(net_header.magic);
    host_header.frame_id = ntohl(net_header.frame_id);
    host_header.payload_size = ntohl(net_header.payload_size);
    host_header.checksum = ntohl(net_header.checksum);

    // 3. Validate
    if (host_header.magic != COBALT_MAGIC) return false;
    if (raw_data.size() < sizeof(PacketHeader) + host_header.payload_size) return false;

    // 4. Extract payload and verify checksum
    out_packet.header = host_header;
    out_packet.payload.resize(host_header.payload_size);
    std::memcpy(out_packet.payload.data(), raw_data.data() + sizeof(PacketHeader), host_header.payload_size);

    uint32_t expected_crc = calculate_crc32(out_packet.payload.data(), out_packet.payload.size());
    if (expected_crc != host_header.checksum) {
        return false; // Data corruption detected!
    }

    return true;
}