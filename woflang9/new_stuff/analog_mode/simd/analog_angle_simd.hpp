#pragma once

#include "analog_modes.hpp"
#include "analog_dot_simd.hpp"
#include "analog_normalize_simd.hpp"
#include "analog_wrap.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- SIMD Angle Between 3D Vectors ---

inline __m256 angle_between_3d(__m256 x1, __m256 y1, __m256 z1,
                               __m256 x2, __m256 y2, __m256 z2) {
    // Normalize both vectors
    __m256 nx1, ny1, nz1;
    __m256 nx2, ny2, nz2;

    normalize_3d(x1, y1, z1, nx1, ny1, nz1);
    normalize_3d(x2, y2, z2, nx2, ny2, nz2);

    // Dot product of normalized vectors
    __m256 dot = dot_product_3d(nx1, ny1, nz1, nx2, ny2, nz2);

    // Clamp dot to valid [-1,1] for acos
    dot = clamp_analog(dot);

    // acos lane-wise
    alignas(32) float vals[8], res[8];
    _mm256_store_ps(vals, dot);
    for (int i = 0; i < 8; ++i) res[i] = acosf(vals[i]);

    __m256 angles = _mm256_load_ps(res);
    return wrap_radians(angles);
}

} // namespace woflang
