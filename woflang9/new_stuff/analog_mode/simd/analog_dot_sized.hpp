#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>

namespace woflang {

// --- SIMD 3D Dot Product ---

inline __m256 dot_product_3d(__m256 x1, __m256 y1, __m256 z1,
                             __m256 x2, __m256 y2, __m256 z2) {
    __m256 prod_x = _mm256_mul_ps(x1, x2);
    __m256 prod_y = _mm256_mul_ps(y1, y2);
    __m256 prod_z = _mm256_mul_ps(z1, z2);

    __m256 sum = _mm256_add_ps(_mm256_add_ps(prod_x, prod_y), prod_z);
    return clamp_analog(sum);
}

} // namespace woflang
