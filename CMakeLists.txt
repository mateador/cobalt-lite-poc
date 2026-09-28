cmake_minimum_required(VERSION 3.15)
project(CobaltLitePoC VERSION 1.0.0 LANGUAGES CXX)

# Enforce modern C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Good practice: Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -pthread)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create the core library (can be linked to both sender and receiver)
add_library(core_lib STATIC
    src/sender.cpp
    src/receiver.cpp
)

# Executables
add_executable(sender_app src/main_sender.cpp)
add_executable(receiver_app src/main_receiver.cpp)

target_link_libraries(sender_app PRIVATE core_lib)
target_link_libraries(receiver_app PRIVATE core_lib)

# NOTE FOR INTERVIEW: 
# To build for Android, we would pass: 
# -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake 
# -DANDROID_ABI=arm64-v8a

# ... [Previous CMake code remains the same] ...

# --- ANDROID / JNI SHARED LIBRARY TARGET ---
# We only build this if we are actually targeting Android, 
# but it's good practice to define it so it compiles on Linux too for testing.
add_library(cobalt_core_jni SHARED
    src/protocol.cpp
    src/network_io.cpp
    src/jni_bridge.cpp       # We will create this next
)

# Link the Android logging library if building for Android
if(ANDROID)
    target_link_libraries(cobalt_core_jni PRIVATE log)
endif()

# Ensure C++17 and warnings apply here too
target_compile_features(cobalt_core_jni PUBLIC cxx_std_17)