#pragma once

#include "analog_modes.hpp"
#include "analog_linear.hpp"
#include "analog_wrap.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Scalar Vector Normalization 2D & 3D ---

inline void normalize_2d(float x, float y, float& x_out, float& y_out) {
    float mag = magnitude_2d(x, y);
    if (mag == 0.0f) {
        x_out = y_out = 0.0f;
    } else {
        x_out = clamp_analog(x / mag);
        y_out = clamp_analog(y / mag);
    }
}

inline void normalize_3d(float x, float y, float z, float& x_out, float& y_out, float& z_out) {
    float mag = magnitude_3d(x, y, z);
    if (mag == 0.0f) {
        x_out = y_out = z_out = 0.0f;
    } else {
        x_out = clamp_analog(x / mag);
        y_out = clamp_analog(y / mag);
        z_out = clamp_analog(z / mag);
    }
}

// --- SIMD Vector Normalization 2D & 3D ---

inline void normalize_2d(__m256 x, __m256 y, __m256& x_out, __m256& y_out) {
    __m256 mag = magnitude_2d(x, y);
    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 mask = _mm256_cmp_ps(mag, zero, _CMP_EQ_OQ);

    x_out = _mm256_div_ps(x, mag);
    y_out = _mm256_div_ps(y, mag);

    x_out = _mm256_blendv_ps(x_out, zero, mask);
    y_out = _mm256_blendv_ps(y_out, zero, mask);

    x_out = clamp_analog(x_out);
    y_out = clamp_analog(y_out);
}

inline void normalize_3d(__m256 x, __m256 y, __m256 z, __m256& x_out, __m256& y_out, __m256& z_out) {
    __m256 mag = magnitude_3d(x, y, z);
    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 mask = _mm256_cmp_ps(mag, zero, _CMP_EQ_OQ);

    x_out = _mm256_div_ps(x, mag);
    y_out = _mm256_div_ps(y, mag);
    z_out = _mm256_div_ps(z, mag);

    x_out = _mm256_blendv_ps(x_out, zero, mask);
    y_out = _mm256_blendv_ps(y_out, zero, mask);
    z_out = _mm256_blendv_ps(z_out, zero, mask);

    x_out = clamp_analog(x_out);
    y_out = clamp_analog(y_out);
    z_out = clamp_analog(z_out);
}

// --- Angle Between Vectors 2D (Scalar) ---

inline float angle_between_2d(float x1, float y1, float x2, float y2) {
    float dot = dot_product_2d(x1, y1, x2, y2);
    float mag1 = magnitude_2d(x1, y1);
    float mag2 = magnitude_2d(x2, y2);
    if (mag1 == 0.0f || mag2 == 0.0f) return 0.0f;
    float cos_theta = clamp_analog(dot / (mag1 * mag2));
    float angle = acosf(cos_theta);
    return wrap_radians(angle);
}

// --- Angle Between Vectors 2D (SIMD) ---

inline __m256 angle_between_2d(__m256 x1, __m256 y1, __m256 x2, __m256 y2) {
    __m256 dot = dot_product_2d(x1, y1, x2, y2);
    __m256 mag1 = magnitude_2d(x1, y1);
    __m256 mag2 = magnitude_2d(x2, y2);
    __m256 prod_mag = _mm256_mul_ps(mag1, mag2);

    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 mask = _mm256_cmp_ps(prod_mag, zero, _CMP_EQ_OQ);

    __m256 cos_theta = _mm256_div_ps(dot, prod_mag);
    cos_theta = clamp_analog(cos_theta);

    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, cos_theta);
    for (int i = 0; i < 8; ++i) res[i] = acosf(vals[i]);
    __m256 angles = _mm256_load_ps(res);

    angles = wrap_radians(angles);
    angles = _mm256_blendv_ps(angles, zero, mask);  // Zero angle if invalid
    return angles;
}

} // namespace woflang
