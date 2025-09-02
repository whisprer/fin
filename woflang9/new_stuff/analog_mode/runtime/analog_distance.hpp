#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Scalar Hypotenuse & Distance ---

inline float analog_hypot(float a, float b) {
    float result = hypotf(a, b);
    return clamp_analog(result);
}

inline float analog_distance_2d(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return analog_hypot(dx, dy);
}

inline float analog_distance_3d(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    float sum = dx * dx + dy * dy + dz * dz;
    float result = sqrtf(sum);
    return clamp_analog(result);
}

// --- SIMD Hypotenuse & Distance (AVX2) ---

inline __m256 analog_hypot(__m256 a, __m256 b) {
    __m256 squared = _mm256_add_ps(_mm256_mul_ps(a, a), _mm256_mul_ps(b, b));
    __m256 result = _mm256_sqrt_ps(squared);
    return clamp_analog(result);
}

inline __m256 analog_distance_2d(__m256 x1, __m256 y1, __m256 x2, __m256 y2) {
    __m256 dx = _mm256_sub_ps(x2, x1);
    __m256 dy = _mm256_sub_ps(y2, y1);
    return analog_hypot(dx, dy);
}

inline __m256 analog_distance_3d(__m256 x1, __m256 y1, __m256 z1, __m256 x2, __m256 y2, __m256 z2) {
    __m256 dx = _mm256_sub_ps(x2, x1);
    __m256 dy = _mm256_sub_ps(y2, y1);
    __m256 dz = _mm256_sub_ps(z2, z1);
    __m256 sum = _mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_add_ps(_mm256_mul_ps(dy, dy), _mm256_mul_ps(dz, dz)));
    __m256 result = _mm256_sqrt_ps(sum);
    return clamp_analog(result);
}

} // namespace woflang
