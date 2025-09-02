#pragma once

#include "analog_modes.hpp"
#include <immintrin.h>

namespace woflang {

// --- Batched 4x4 Matrix Multiply: Processes 8 parallel transforms ---

inline void multiply_4x4_batch(const float* a, const float* b, float* result) {
    // a, b, result are arrays of 16 floats * 8 lanes = 128 floats each
    // Layout: row-major, so a[ lane*16 + row*4 + col ]

    for (int lane = 0; lane < 8; ++lane) {
        const float* mA = &a[lane * 16];
        const float* mB = &b[lane * 16];
        float* mR = &result[lane * 16];

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += mA[r * 4 + k] * mB[k * 4 + c];
                }
                mR[r * 4 + c] = clamp_analog(sum);
            }
        }
    }
}

} // namespace woflang
