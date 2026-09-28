#include <jni.h>
#include <string>
#include <android/log.h> // Android native logging
#include "protocol.hpp"
#include "network_io.hpp"

#define LOG_TAG "CobaltLiteJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

// This function name MUST match the Java/Kotlin package and class structure.
// e.g., com.vncautomotive.cobaltlite.NativeBridge.initializeProtocol()
JNIEXPORT jboolean JNICALL
Java_com_vncautomotive_cobaltlite_NativeBridge_initializeProtocol(JNIEnv* env, jobject /* this */) {
    LOGI("Initializing Cobalt Lite Protocol from C++ via JNI");
    
    // Here you would initialize sockets, start the worker threads, etc.
    // For the PoC, we just verify the CRC32 logic works on the device.
    
    uint8_t test_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    uint32_t crc = calculate_crc32(test_data, 5);
    
    LOGI("Calculated CRC32 on Android device: 0x%08X", crc);
    
    return JNI_TRUE; // Success
}

JNIEXPORT jstring JNICALL
Java_com_vncautomotive_cobaltlite_NativeBridge_getProtocolVersion(JNIEnv* env, jobject /* this */) {
    return env->NewStringUTF("Cobalt Lite PoC v1.0 - C++17 / JNI");
}

} // extern "C"
