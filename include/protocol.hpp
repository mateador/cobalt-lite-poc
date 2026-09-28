#pragma once
#include <cstdint>
#include <vector>
#include "frame.hpp"

// Magic number to identify valid packets (e.g., 'C', 'O', 'B', 'A')
constexpr uint32_t COBALT_MAGIC = 0x41424F43; 

// Packed struct ensures no compiler padding. Total size = 16 bytes.
#pragma pack(push, 1)
struct PacketHeader {
    uint32_t magic;          // 4 bytes: Validation
    uint32_t frame_id;       // 4 bytes: For ordering/deduplication
    uint32_t payload_size;   // 4 bytes: Size of the following data
    uint32_t checksum;       // 4 bytes: CRC32 of the payload
};
#pragma pack(pop)

struct NetworkPacket {
    PacketHeader header;
    std::vector<uint8_t> payload;
};

// Function declarations
uint32_t calculate_crc32(const uint8_t* data, size_t length);
std::vector<uint8_t> serialize_packet(const NetworkPacket& packet);
bool deserialize_packet(const std::vector<uint8_t>& raw_data, NetworkPacket& out_packet);