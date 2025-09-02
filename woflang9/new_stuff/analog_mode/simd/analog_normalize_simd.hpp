#pragma once

#include "analog_modes.hpp"
#include "analog_vector_simd.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- SIMD Normalize 3D Vector ---

inline void normalize_3d(__m256 x, __m256 y, __m256 z,
                         __m256& x_out, __m256& y_out, __m256& z_out) {
    __m256 mag_sq = _mm256_add_ps(
                        _mm256_add_ps(_mm256_mul_ps(x, x), _mm256_mul_ps(y, y)),
                        _mm256_mul_ps(z, z)
                    );

    __m256 mag = _mm256_sqrt_ps(mag_sq);
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

} // namespace woflang
