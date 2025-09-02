#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>

namespace woflang {

// --- Apply 3x3 Transform to 2D Vectors (8 parallel lanes) ---

inline void apply_transform_2d_batch(const float* matrices, const __m256 x, const __m256 y,
                                     __m256& x_out, __m256& y_out) {
    alignas(32) float xs[8], ys[8];
    _mm256_store_ps(xs, x);
    _mm256_store_ps(ys, y);

    float xr[8], yr[8];

    for (int lane = 0; lane < 8; ++lane) {
        const float* m = &matrices[lane * 9];

        float x_in = xs[lane];
        float y_in = ys[lane];

        xr[lane] = clamp_analog(m[0] * x_in + m[1] * y_in + m[2]);
        yr[lane] = clamp_analog(m[3] * x_in + m[4] * y_in + m[5]);
        // Ignore row 3 since homogeneous coordinates implied w=1
    }

    x_out = _mm256_load_ps(xr);
    y_out = _mm256_load_ps(yr);
}

} // namespace woflang
