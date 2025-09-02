#pragma once

#include "analog_modes.hpp"
#include "analog_linear.hpp"
#include "analog_vector_ops.hpp"
#include <immintrin.h>
#include <cmath>

namespace woflang {

// --- Scalar Projection of a onto b (2D) ---

inline void project_vector_2d(float ax, float ay, float bx, float by, float& proj_x, float& proj_y) {
    float dot_ab = dot_product_2d(ax, ay, bx, by);
    float mag_b_squared = bx * bx + by * by;

    if (mag_b_squared == 0.0f) {
        proj_x = proj_y = 0.0f;
    } else {
        float scale = dot_ab / mag_b_squared;
        proj_x = clamp_analog(scale * bx);
        proj_y = clamp_analog(scale * by);
    }
}

// --- Scalar Projection of a onto b (3D) ---

inline void project_vector_3d(float ax, float ay, float az, float bx, float by, float bz, float& proj_x, float& proj_y, float& proj_z) {
    float dot_ab = dot_product_3d(ax, ay, az, bx, by, bz);
    float mag_b_squared = bx * bx + by * by + bz * bz;

    if (mag_b_squared == 0.0f) {
        proj_x = proj_y = proj_z = 0.0f;
    } else {
        float scale = dot_ab / mag_b_squared;
        proj_x = clamp_analog(scale * bx);
        proj_y = clamp_analog(scale * by);
        proj_z = clamp_analog(scale * bz);
    }
}

// --- SIMD Projection of a onto b (2D, AVX2) ---

inline void project_vector_2d(__m256 ax, __m256 ay, __m256 bx, __m256 by, __m256& proj_x, __m256& proj_y) {
    __m256 dot_ab = dot_product_2d(ax, ay, bx, by);
    __m256 mag_b_squared = _mm256_add_ps(_mm256_mul_ps(bx, bx), _mm256_mul_ps(by, by));

    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 mask = _mm256_cmp_ps(mag_b_squared, zero, _CMP_EQ_OQ);

    __m256 scale = _mm256_div_ps(dot_ab, mag_b_squared);

    proj_x = _mm256_mul_ps(scale, bx);
    proj_y = _mm256_mul_ps(scale, by);

    proj_x = _mm256_blendv_ps(proj_x, zero, mask);
    proj_y = _mm256_blendv_ps(proj_y, zero, mask);

    proj_x = clamp_analog(proj_x);
    proj_y = clamp_analog(proj_y);
}

// --- SIMD Projection of a onto b (3D, AVX2) ---

inline void project_vector_3d(__m256 ax, __m256 ay, __m256 az, __m256 bx, __m256 by, __m256 bz, __m256& proj_x, __m256& proj_y, __m256& proj_z) {
    __m256 dot_ab = dot_product_3d(ax, ay, az, bx, by, bz);
    __m256 mag_b_squared = _mm256_add_ps(
                               _mm256_add_ps(_mm256_mul_ps(bx, bx), _mm256_mul_ps(by, by)),
                               _mm256_mul_ps(bz, bz)
                           );

    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 mask = _mm256_cmp_ps(mag_b_squared, zero, _CMP_EQ_OQ);

    __m256 scale = _mm256_div_ps(dot_ab, mag_b_squared);

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
