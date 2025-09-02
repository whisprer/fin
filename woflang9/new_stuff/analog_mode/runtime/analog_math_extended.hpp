#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Scalar Analog Math Extended ---

inline float analog_negate(float a) {
    float result = -a;
    return clamp_analog(result);
}

inline float analog_abs(float a) {
    float result = fabsf(a);
    return clamp_analog(result);
}

inline float analog_sqrt(float a) {
    if (a < 0.0f) return clamp_analog(0.0f);  // sqrt of negative is zero for analog domain
    float result = sqrtf(a);
    return clamp_analog(result);
}

inline float analog_pow(float base, float exponent) {
    float result = powf(base, exponent);
    return clamp_analog(result);
}

inline float analog_sin(float a) {
    float result = sinf(a);
    return clamp_analog(result);
}

inline float analog_cos(float a) {
    float result = cosf(a);
    return clamp_analog(result);
}

// --- SIMD Analog Math Extended (AVX2) ---

inline __m256 analog_negate(__m256 a) {
    __m256 result = _mm256_sub_ps(_mm256_set1_ps(0.0f), a);
    return clamp_analog(result);
}

inline __m256 analog_abs(__m256 a) {
    __m256 mask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
    __m256 result = _mm256_and_ps(a, mask);
    return clamp_analog(result);
}

inline __m256 analog_sqrt(__m256 a) {
    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 safe_a = _mm256_max_ps(a, zero);
    __m256 result = _mm256_sqrt_ps(safe_a);
    return clamp_analog(result);
}

inline __m256 analog_pow(__m256 base, __m256 exponent) {
    alignas(32) float b[8], e[8], r[8];
    _mm256_store_ps(b, base);
    _mm256_store_ps(e, exponent);
    for (int i = 0; i < 8; ++i) r[i] = powf(b[i], e[i]);
    __m256 result = _mm256_load_ps(r);
    return clamp_analog(result);
}

inline __m256 analog_sin(__m256 a) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, a);
    for (int i = 0; i < 8; ++i) res[i] = sinf(vals[i]);
    __m256 result = _mm256_load_ps(res);
    return clamp_analog(result);
}

inline __m256 analog_cos(__m256 a) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, a);
    for (int i = 0; i < 8; ++i) res[i] = cosf(vals[i]);
    __m256 result = _mm256_load_ps(res);
    return clamp_analog(result);
}

} // namespace woflang
