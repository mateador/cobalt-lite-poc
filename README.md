# Cobalt Lite PoC

A lightweight, cross-platform C++17 prototype demonstrating low-latency frame streaming over TCP. This project was built to mirror the connectivity and telemetry challenges solved by in-vehicle infotainment solutions (like VNC Automotive's Cobalt product line).

## 🚀 Overview

This repository contains a multi-threaded Producer-Consumer application that simulates capturing, packaging, and transmitting video frames over a network. 
- **Sender**: Generates frames, pushes them to a thread-safe queue, and transmits them over TCP using a custom binary protocol.
- **Receiver**: Connects via TCP, handles partial reads, reassembles packets, validates data integrity via CRC32, and reports live metrics.

## 🛠️ Key Technical Features

| Domain | Implementation Details |
| :--- | :--- |
| **Concurrency** | Multi-threaded Producer-Consumer pattern using `std::mutex` and `std::condition_variable` to prevent busy-waiting. |
| **Network Protocol** | Custom 16-byte binary header (Magic Number, Frame ID, Payload Size, CRC32). |
| **Data Integrity** | CRC32 checksums to detect corruption over noisy networks (critical for automotive safety). |
| **Cross-Platform** | Strict endianness handling (`htonl`/`ntohl`) and `#pragma pack(1)` to ensure compatibility between x86 and ARM architectures. |
| **TCP Robustness** | Custom `read_exact` and `send_all` loops to handle TCP stream fragmentation and partial reads. |
| **Memory Safety** | Move-only semantics for large frame buffers (`std::unique_ptr`, `std::move`) to eliminate unnecessary memory copying. |
| **Build System** | Modern CMake (C++17) configured for both native Linux/macOS execution and cross-compilation via the Android NDK. |

## 💻 Quick Start (Native Linux / macOS)

```bash
# 1. Clone and navigate to the project
git clone <https://github.com/mateador/cobalt-lite-poc>
cd cobalt-lite-poc

# 2. Build the project
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# 3. Run the applications (open two separate terminals)
# Terminal 1: Start the sender
./sender

# Terminal 2: Start the receiver
./receiver
```

## 📱 Android / AAOS Cross-Compilation

The core C++ logic is designed to be compiled as a shared library (`.so`) for integration into Android Automotive OS (AAOS) via JNI.

```bash
# Ensure ANDROID_NDK environment variable is set
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-28 \
      -B build-android

cmake --build build-android
```

## 🔮 Future Roadmap

- [ ] **Video Compression:** Integrate FFmpeg (H.264/H.265) to drastically reduce bandwidth.
- [ ] **Security:** Implement TLS 1.3 for encrypted fleet remote access.
- [ ] **Lossy Networks:** Add UDP support with Forward Error Correction (FEC) for unstable Wi-Fi/5G connections.
- [ ] **Hardware Acceleration:** Implement V4L2 (Linux) or Android MediaCodec for zero-copy hardware frame capture.

## 📄 License

MIT License