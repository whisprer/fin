#pragma once

#include "analog_modes.hpp"
#include "analog_matrix.hpp"
#include "analog_wrap.hpp"
#include "analog_linear.hpp"
#include <cmath>

namespace woflang {

// --- Build 3x3 2D Transform Matrix (Rotation + Translation) ---

inline Matrix3x3 transform_matrix_2d(float angle_radians, float translate_x, float translate_y) {
    Matrix3x3 mat;
    float c = clamp_analog(cosf(angle_radians));
    float s = clamp_analog(sinf(angle_radians));

    mat.m00 = c;
    mat.m01 = -s;
    mat.m02 = clamp_analog(translate_x);

    mat.m10 = s;
    mat.m11 = c;
    mat.m12 = clamp_analog(translate_y);

    mat.m20 = 0.0f;
    mat.m21 = 0.0f;
    mat.m22 = 1.0f;

    return mat;
}

// --- Apply 3x3 Transform to 2D Vector (Assumes z=1 for translation) ---

inline void apply_transform_2d(const Matrix3x3& mat, float x, float y, float& x_out, float& y_out) {
    x_out = clamp_analog(mat.m00 * x + mat.m01 * y + mat.m02);
    y_out = clamp_analog(mat.m10 * x + mat.m11 * y + mat.m12);
}

// --- Apply 3x3 Transform to 2D Vector without translation ---

inline void apply_rotation_2d(const Matrix3x3& mat, float x, float y, float& x_out, float& y_out) {
    x_out = clamp_analog(mat.m00 * x + mat.m01 * y);
    y_out = clamp_analog(mat.m10 * x + mat.m11 * y);
}

} // namespace woflang
