#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Scalar Dot Product 2D & 3D ---

inline float dot_product_2d(float x1, float y1, float x2, float y2) {
    float result = (x1 * x2) + (y1 * y2);
    return clamp_analog(result);
}

inline float dot_product_3d(float x1, float y1, float z1, float x2, float y2, float z2) {
    float result = (x1 * x2) + (y1 * y2) + (z1 * z2);
    return clamp_analog(result);
}

// --- SIMD Dot Product 2D (AVX2) ---

inline __m256 dot_product_2d(__m256 x1, __m256 y1, __m256 x2, __m256 y2) {
    __m256 prod_x = _mm256_mul_ps(x1, x2);
    __m256 prod_y = _mm256_mul_ps(y1, y2);
    __m256 result = _mm256_add_ps(prod_x, prod_y);
    return clamp_analog(result);
}

// --- SIMD Dot Product 3D (AVX2) ---

inline __m256 dot_product_3d(__m256 x1, __m256 y1, __m256 z1, __m256 x2, __m256 y2, __m256 z2) {
    __m256 prod_x = _mm256_mul_ps(x1, x2);
    __m256 prod_y = _mm256_mul_ps(y1, y2);
    __m256 prod_z = _mm256_mul_ps(z1, z2);
    __m256 sum_xy = _mm256_add_ps(prod_x, prod_y);
    __m256 result = _mm256_add_ps(sum_xy, prod_z);
    return clamp_analog(result);
}

// --- Scalar Magnitude (Vector Length) 2D & 3D ---

inline float magnitude_2d(float x, float y) {
    float result = hypotf(x, y);
    return clamp_analog(result);
}

inline float magnitude_3d(float x, float y, float z) {
    float sum = (x * x) + (y * y) + (z * z);
    float result = sqrtf(sum);
    return clamp_analog(result);
}

// --- SIMD Magnitude 2D & 3D (AVX2) ---

inline __m256 magnitude_2d(__m256 x, __m256 y) {
    __m256 sum = _mm256_add_ps(_mm256_mul_ps(x, x), _mm256_mul_ps(y, y));
    __m256 result = _mm256_sqrt_ps(sum);
    return clamp_analog(result);
}

inline __m256 magnitude_3d(__m256 x, __m256 y, __m256 z) {
    __m256 sum = _mm256_add_ps(
                    _mm256_add_ps(_mm256_mul_ps(x, x), _mm256_mul_ps(y, y)),
                    _mm256_mul_ps(z, z)
                 );
    __m256 result = _mm256_sqrt_ps(sum);
    return clamp_analog(result);
}

} // namespace woflang
