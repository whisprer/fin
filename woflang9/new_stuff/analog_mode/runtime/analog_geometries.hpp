#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Scalar Geometry & Trig ---

inline float analog_tan(float a) {
    float result = tanf(a);
    return clamp_analog(result);
}

inline float analog_asin(float a) {
    float clamped = clamp_analog(a);
    float result = asinf(clamped);  // Clamp input for domain safety
    return clamp_analog(result);
}

inline float analog_acos(float a) {
    float clamped = clamp_analog(a);
    float result = acosf(clamped);
    return clamp_analog(result);
}

inline float analog_atan(float a) {
    float result = atanf(a);
    return clamp_analog(result);
}

inline float analog_atan2(float y, float x) {
    float result = atan2f(y, x);
    return clamp_analog(result);
}

// --- SIMD Geometry & Trig (AVX2) ---

inline __m256 analog_tan(__m256 a) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, a);
    for (int i = 0; i < 8; ++i) res[i] = tanf(vals[i]);
    __m256 result = _mm256_load_ps(res);
    return clamp_analog(result);
}

inline __m256 analog_asin(__m256 a) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, a);
    for (int i = 0; i < 8; ++i) res[i] = asinf(clamp_analog(vals[i]));
    __m256 result = _mm256_load_ps(res);
    return clamp_analog(result);
}

inline __m256 analog_acos(__m256 a) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, a);
    for (int i = 0; i < 8; ++i) res[i] = acosf(clamp_analog(vals[i]));
    __m256 result = _mm256_load_ps(res);
    return clamp_analog(result);
}

inline __m256 analog_atan(__m256 a) {
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, a);
    for (int i = 0; i < 8; ++i) res[i] = atanf(vals[i]);
    __m256 result = _mm256_load_ps(res);
    return clamp_analog(result);
}

inline __m256 analog_atan2(__m256 y, __m256 x) {
    alignas(32) float vy[8], vx[8], res[8];
    _mm256_store_ps(vy, y);
    _mm256_store_ps(vx, x);
    for (int i = 0; i < 8; ++i) res[i] = atan2f(vy[i], vx[i]);
    __m256 result = _mm256_load_ps(res);
    return clamp_analog(result);
}

} // namespace woflang
