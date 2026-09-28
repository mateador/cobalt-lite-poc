#pragma once
#include <vector>
#include <cstdint>

// Returns true if all bytes were sent, false on error/disconnect
bool send_all(int socket_fd, const std::vector<uint8_t>& data);

// Returns true if exactly 'size' bytes were read, false on error/disconnect
bool read_exact(int socket_fd, std::vector<uint8_t>& buffer, size_t size);