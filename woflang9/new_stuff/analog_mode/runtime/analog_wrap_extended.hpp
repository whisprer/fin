#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Constants ---
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float HALF_CIRCLE_DEG = 180.0f;
constexpr float FULL_CIRCLE_DEG = 360.0f;

// --- Scalar Symmetric Angle Wrapping ---

inline float wrap_radians_symmetric(float radians) {
    float wrapped = fmodf(radians + PI, TWO_PI);
    if (wrapped < 0.0f) wrapped += TWO_PI;
    wrapped -= PI;
    return clamp_analog(wrapped);
}

inline float wrap_degrees_symmetric(float degrees) {
    float wrapped = fmodf(degrees + HALF_CIRCLE_DEG, FULL_CIRCLE_DEG);
    if (wrapped < 0.0f) wrapped += FULL_CIRCLE_DEG;
    wrapped -= HALF_CIRCLE_DEG;
    return clamp_analog(wrapped);
}

// --- SIMD Symmetric Angle Wrapping (AVX2) ---

inline __m256 wrap_radians_symmetric(__m256 radians) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, radians);
    for (int i = 0; i < 8; ++i) {
        float wrapped = fmodf(vals[i] + PI, TWO_PI);
        if (wrapped < 0.0f) wrapped += TWO_PI;
        wrapped -= PI;
        res[i] = clamp_analog(wrapped);
    }
    return _mm256_load_ps(res);
}

inline __m256 wrap_degrees_symmetric(__m256 degrees) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, degrees);
    for (int i = 0; i < 8; ++i) {
        float wrapped = fmodf(vals[i] + HALF_CIRCLE_DEG, FULL_CIRCLE_DEG);
        if (wrapped < 0.0f) wrapped += FULL_CIRCLE_DEG;
        wrapped -= HALF_CIRCLE_DEG;
        res[i] = clamp_analog(wrapped);
    }
    return _mm256_load_ps(res);
}

} // namespace woflang
