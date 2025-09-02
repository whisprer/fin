#pragma once

#include "analog_modes.hpp"
#include "analog_matrix.hpp"

namespace woflang {

// --- Chain Multiply Two 3x3 Matrices (a * b) ---

inline Matrix3x3 multiply_chain(const Matrix3x3& a, const Matrix3x3& b) {
    Matrix3x3 result;

    result.m00 = clamp_analog(a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20);
    result.m01 = clamp_analog(a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21);
    result.m02 = clamp_analog(a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22);

    result.m10 = clamp_analog(a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20);
    result.m11 = clamp_analog(a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21);
    result.m12 = clamp_analog(a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22);

    result.m20 = clamp_analog(a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20);
    result.m21 = clamp_analog(a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21);
    result.m22 = clamp_analog(a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22);

    return result;
}

// --- Chain Multiply Arbitrary Count of 3x3 Matrices ---

template <typename... Matrices>
Matrix3x3 multiply_chain(const Matrix3x3& first, const Matrix3x3& second, const Matrices&... rest) {
    return multiply_chain(multiply_chain(first, second), rest...);
}

} // namespace woflang
