#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>

namespace woflang {

// --- Apply 4x4 Transform to 3D Vectors (8 parallel lanes) ---

inline void apply_transform_3d_batch(const float* matrices, const __m256 x, const __m256 y, const __m256 z,
                                     __m256& x_out, __m256& y_out, __m256& z_out) {
    alignas(32) float xs[8], ys[8], zs[8];
    _mm256_store_ps(xs, x);
    _mm256_store_ps(ys, y);
    _mm256_store_ps(zs, z);

    float xr[8], yr[8], zr[8];

    for (int lane = 0; lane < 8; ++lane) {
        const float* m = &matrices[lane * 16];

        float x_in = xs[lane];
        float y_in = ys[lane];
        float z_in = zs[lane];

        xr[lane] = clamp_analog(m[0] * x_in + m[1] * y_in + m[2] * z_in + m[3]);
        yr[lane] = clamp_analog(m[4] * x_in + m[5] * y_in + m[6] * z_in + m[7]);
        zr[lane] = clamp_analog(m[8] * x_in + m[9] * y_in + m[10] * z_in + m[11]);
    }

    x_out = _mm256_load_ps(xr);
    y_out = _mm256_load_ps(yr);
    z_out = _mm256_load_ps(zr);
}

} // namespace woflang
