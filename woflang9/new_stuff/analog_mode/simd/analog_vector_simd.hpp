#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- SIMD Vector Add 3D ---

inline void vector_add_3d(__m256 x1, __m256 y1, __m256 z1, __m256 x2, __m256 y2, __m256 z2,
                          __m256& x_out, __m256& y_out, __m256& z_out) {
    x_out = clamp_analog(_mm256_add_ps(x1, x2));
    y_out = clamp_analog(_mm256_add_ps(y1, y2));
    z_out = clamp_analog(_mm256_add_ps(z1, z2));
}

// --- SIMD Vector Subtract 3D ---

inline void vector_sub_3d(__m256 x1, __m256 y1, __m256 z1, __m256 x2, __m256 y2, __m256 z2,
                          __m256& x_out, __m256& y_out, __m256& z_out) {
    x_out = clamp_analog(_mm256_sub_ps(x1, x2));
    y_out = clamp_analog(_mm256_sub_ps(y1, y2));
    z_out = clamp_analog(_mm256_sub_ps(z1, z2));
}

// --- SIMD Scale Vector 3D ---

inline void vector_scale_3d(__m256 x, __m256 y, __m256 z, __m256 scalar,
                            __m256& x_out, __m256& y_out, __m256& z_out) {
    x_out = clamp_analog(_mm256_mul_ps(x, scalar));
    y_out = clamp_analog(_mm256_mul_ps(y, scalar));
    z_out = clamp_analog(_mm256_mul_ps(z, scalar));
}

} // namespace woflang
