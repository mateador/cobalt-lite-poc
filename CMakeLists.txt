cmake_minimum_required(VERSION 3.15)
project(CobaltLitePoC VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library: protocol + network I/O (platform-agnostic)
add_library(cobalt_core STATIC
    src/protocol.cpp
    src/network_io.cpp
)
target_link_libraries(cobalt_core PUBLIC pthread)

# --- LOCAL EXECUTABLES (Linux / macOS) ---
if(NOT ANDROID)
    add_executable(sender_app src/main_sender.cpp)

    target_link_libraries(sender PRIVATE cobalt_core)

    add_executable(receiver_app src/main_receiver.cpp)
    target_link_libraries(receiver PRIVATE cobalt_core)
endif()

# --- ANDROID JNI LIBRARY (only when cross-compiling with NDK) ---
if(ANDROID)
    add_library(cobalt_core_jni SHARED
        src/jni_bridge.cpp
    )
    target_link_libraries(cobalt_core_jni PRIVATE cobalt_core log)
endif()