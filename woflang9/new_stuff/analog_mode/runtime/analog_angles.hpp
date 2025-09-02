#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Constants ---
constexpr float PI = 3.14159265358979323846f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

// --- Scalar Conversions ---

inline float degrees_to_radians(float degrees) {
    float result = degrees * DEG_TO_RAD;
    return clamp_analog(result);
}

inline float radians_to_degrees(float radians) {
    float result = radians * RAD_TO_DEG;
    return clamp_analog(result);
}

// --- SIMD Conversions (AVX2) ---

inline __m256 degrees_to_radians(__m256 degrees) {
    __m256 factor = _mm256_set1_ps(DEG_TO_RAD);
    __m256 result = _mm256_mul_ps(degrees, factor);
    return clamp_analog(result);
}

inline __m256 radians_to_degrees(__m256 radians) {
    __m256 factor = _mm256_set1_ps(RAD_TO_DEG);
    __m256 result = _mm256_mul_ps(radians, factor);
    return clamp_analog(result);
}

} // namespace woflang
