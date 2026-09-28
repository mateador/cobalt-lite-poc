#include "network_io.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>

bool send_all(int socket_fd, const std::vector<uint8_t>& data) {
    size_t total_sent = 0;
    while (total_sent < data.size()) {
        ssize_t bytes_sent = send(socket_fd, data.data() + total_sent, data.size() - total_sent, MSG_NOSIGNAL);
        if (bytes_sent <= 0) {
            return false; // Error or connection closed
        }
        total_sent += bytes_sent;
    }
    return true;
}

bool read_exact(int socket_fd, std::vector<uint8_t>& buffer, size_t size) {
    buffer.resize(size);
    size_t total_read = 0;
    while (total_read < size) {
        ssize_t bytes_read = recv(socket_fd, buffer.data() + total_read, size - total_read, 0);
        if (bytes_read <= 0) {
            return false; // Error or connection closed
        }
        total_read += bytes_read;
    }
    return true;
}