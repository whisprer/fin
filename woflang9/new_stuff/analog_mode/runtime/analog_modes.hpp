#pragma once

#include <immintrin.h>
#include <cstdint>
#include <cmath>
#include <stdexcept>

namespace woflang {

// --- Analog Mode Definition ---
enum class AnalogMode : uint8_t {
    INT_201,         // -100 to +100 inclusive, int
    INT_2001,        // -1000 to +1000 inclusive, int
    FLOAT_UNIT,      // -1.0 to +1.0 float
    FLOAT_CUSTOM     // user-defined float min/max
};

// --- Analog State ---
struct AnalogConfig {
    AnalogMode mode;
    float custom_min;
    float custom_max;

    AnalogConfig() : mode(AnalogMode::INT_201), custom_min(-1.0f), custom_max(1.0f) {}
};

inline AnalogConfig analog_state;

// --- Range Getters ---
inline float analog_min() {
    switch (analog_state.mode) {
        case AnalogMode::INT_201: return -100.0f;
        case AnalogMode::INT_2001: return -1000.0f;
        case AnalogMode::FLOAT_UNIT: return -1.0f;
        case AnalogMode::FLOAT_CUSTOM: return analog_state.custom_min;
    }
    throw std::runtime_error("Unknown analog mode");
}

inline float analog_max() {
    switch (analog_state.mode) {
        case AnalogMode::INT_201: return 100.0f;
        case AnalogMode::INT_2001: return 1000.0f;
        case AnalogMode::FLOAT_UNIT: return 1.0f;
        case AnalogMode::FLOAT_CUSTOM: return analog_state.custom_max;
    }
    throw std::runtime_error("Unknown analog mode");
}

// --- Clamping Utilities ---
inline float clamp_analog(float value) {
    float min_val = analog_min();
    float max_val = analog_max();
    return fmaxf(min_val, fminf(value, max_val));
}

inline __m256 clamp_analog(__m256 value) {
    __m256 min_val = _mm256_set1_ps(analog_min());
    __m256 max_val = _mm256_set1_ps(analog_max());
    value = _mm256_max_ps(value, min_val);
    value = _mm256_min_ps(value, max_val);
    return value;
}

// --- Setting Modes ---
inline void set_analog_mode(AnalogMode mode, float custom_min = -1.0f, float custom_max = 1.0f) {
    analog_state.mode = mode;
    if (mode == AnalogMode::FLOAT_CUSTOM) {
        if (custom_min >= custom_max) {
            throw std::invalid_argument("Custom analog range must have min < max");
        }
        analog_state.custom_min = custom_min;
        analog_state.custom_max = custom_max;
    }
}

} // namespace woflang
