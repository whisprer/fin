#pragma once

#include "analog_modes.hpp"
#include "analog_matrix.hpp"
#include "analog_wrap.hpp"
#include "analog_linear.hpp"
#include <cmath>

namespace woflang {

// --- 2D Rotation Matrix from Angle (radians) ---

inline Matrix2x2 rotation_matrix_2d(float angle_radians) {
    Matrix2x2 rot;
    float c = clamp_analog(cosf(angle_radians));
    float s = clamp_analog(sinf(angle_radians));

    rot.m00 = c;
    rot.m01 = -s;
    rot.m10 = s;
    rot.m11 = c;

    return rot;
}

// --- 2D Rotate Vector by Angle ---

inline void rotate_vector_2d(float x, float y, float angle_radians, float& x_out, float& y_out) {
    Matrix2x2 rot = rotation_matrix_2d(angle_radians);
    x_out = clamp_analog(rot.m00 * x + rot.m01 * y);
    y_out = clamp_analog(rot.m10 * x + rot.m11 * y);
}

// --- 2D Rotate Vector Using Existing Matrix ---

inline void rotate_vector_2d(const Matrix2x2& rot, float x, float y, float& x_out, float& y_out) {
    x_out = clamp_analog(rot.m00 * x + rot.m01 * y);
    y_out = clamp_analog(rot.m10 * x + rot.m11 * y);
}

} // namespace woflang
