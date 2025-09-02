#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Scalar Analog Math ---
inline float analog_add(float a, float b) {
    float result = a + b;
    return clamp_analog(result);
}

inline float analog_sub(float a, float b) {
    float result = a - b;
    return clamp_analog(result);
}

inline float analog_mul(float a, float b) {
    float result = a * b;
    return clamp_analog(result);
}

inline float analog_div(float a, float b) {
    if (b == 0.0f) return clamp_analog(0.0f);  // Safe div by zero fallback
    float result = a / b;
    return clamp_analog(result);
}

// --- SIMD Analog Math (AVX2, 256-bit) ---
inline __m256 analog_add(__m256 a, __m256 b) {
    __m256 result = _mm256_add_ps(a, b);
    return clamp_analog(result);
}

inline __m256 analog_sub(__m256 a, __m256 b) {
    __m256 result = _mm256_sub_ps(a, b);
    return clamp_analog(result);
}

inline __m256 analog_mul(__m256 a, __m256 b) {
    __m256 result = _mm256_mul_ps(a, b);
    return clamp_analog(result);
}

inline __m256 analog_div(__m256 a, __m256 b) {
    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 safe_b = _mm256_blendv_ps(b, _mm256_set1_ps(1.0f), _mm256_cmp_ps(b, zero, _CMP_EQ_OQ));  // Avoid div by 0
    __m256 result = _mm256_div_ps(a, safe_b);
    return clamp_analog(result);
}

} // namespace woflang
