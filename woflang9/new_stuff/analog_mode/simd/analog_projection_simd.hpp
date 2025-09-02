#pragma once

#include "analog_modes.hpp"
#include "analog_dot_simd.hpp"
#include <immintrin.h>

namespace woflang {

// --- SIMD 3D Vector Projection of a onto b ---

inline void project_vector_3d(__m256 ax, __m256 ay, __m256 az,
                              __m256 bx, __m256 by, __m256 bz,
                              __m256& proj_x, __m256& proj_y, __m256& proj_z) {
    __m256 dot_ab = dot_product_3d(ax, ay, az, bx, by, bz);
    __m256 mag_b_sq = _mm256_add_ps(
                          _mm256_add_ps(_mm256_mul_ps(bx, bx), _mm256_mul_ps(by, by)),
                          _mm256_mul_ps(bz, bz)
                      );

    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 mask = _mm256_cmp_ps(mag_b_sq, zero, _CMP_EQ_OQ);

    __m256 scale = _mm256_div_ps(dot_ab, mag_b_sq);

    proj_x = _mm256_mul_ps(scale, bx);
    proj_y = _mm256_mul_ps(scale, by);
    proj_z = _mm256_mul_ps(scale, bz);

    proj_x = _mm256_blendv_ps(proj_x, zero, mask);
    proj_y = _mm256_blendv_ps(proj_y, zero, mask);
    proj_z = _mm256_blendv_ps(proj_z, zero, mask);

    proj_x = clamp_analog(proj_x);
    proj_y = clamp_analog(proj_y);
    proj_z = clamp_analog(proj_z);
}

} // namespace woflang
