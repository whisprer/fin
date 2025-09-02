#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Polar to Cartesian (2D) ---

inline void polar_to_cartesian(float radius, float angle_radians, float& x_out, float& y_out) {
    x_out = clamp_analog(radius * cosf(angle_radians));
    y_out = clamp_analog(radius * sinf(angle_radians));
}

// --- Cartesian to Polar (2D) ---

inline void cartesian_to_polar(float x, float y, float& radius_out, float& angle_radians_out) {
    radius_out = clamp_analog(hypotf(x, y));
    angle_radians_out = wrap_radians(atan2f(y, x));
}

// --- SIMD Polar to Cartesian (2D, AVX2) ---

inline void polar_to_cartesian(__m256 radius, __m256 angle_radians, __m256& x_out, __m256& y_out) {
    alignas(32) float angles[8], rads[8], xs[8], ys[8];
    _mm256_store_ps(angles, angle_radians);
    _mm256_store_ps(rads, radius);
    for (int i = 0; i < 8; ++i) {
        xs[i] = clamp_analog(rads[i] * cosf(angles[i]));
        ys[i] = clamp_analog(rads[i] * sinf(angles[i]));
    }
    x_out = _mm256_load_ps(xs);
    y_out = _mm256_load_ps(ys);
}

// --- SIMD Cartesian to Polar (2D, AVX2) ---

inline void cartesian_to_polar(__m256 x, __m256 y, __m256& radius_out, __m256& angle_radians_out) {
    alignas(32) float xs[8], ys[8], rads[8], angles[8];
    _mm256_store_ps(xs, x);
    _mm256_store_ps(ys, y);
    for (int i = 0; i < 8; ++i) {
        rads[i] = clamp_analog(hypotf(xs[i], ys[i]));
        angles[i] = wrap_radians(atan2f(ys[i], xs[i]));
    }
    radius_out = _mm256_load_ps(rads);
    angle_radians_out = _mm256_load_ps(angles);
}

} // namespace woflang
