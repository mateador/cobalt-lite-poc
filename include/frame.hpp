#pragma once
#include <vector>
#include <cstdint>
#include <chrono>

struct Frame {
    uint32_t width;
    uint32_t height;
    uint64_t timestamp_ms;
    std::vector<uint8_t> payload;

    // Default constructor
    Frame() : width(0), height(0), timestamp_ms(0) {}

    // Parameterized constructor
    Frame(uint32_t w, uint32_t h, std::vector<uint8_t> data) 
        : width(w), height(h), payload(std::move(data)) { // <-- Move semantics!
        timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    // Delete copy constructor/assignment to enforce move-only semantics 
    // (Optional but highly impressive for performance-critical roles)
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

    // Enable move constructor and assignment
    Frame(Frame&&) noexcept = default;
    Frame& operator=(Frame&&) noexcept = default;
};