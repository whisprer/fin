#pragma once

#include "analog_modes.hpp"
#include <cmath>

namespace woflang {

// --- 4x4 Matrix Struct for 3D Transforms ---

struct Matrix4x4 {
    float m[4][4];

    Matrix4x4() {
        // Identity by default
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                m[r][c] = (r == c) ? 1.0f : 0.0f;
    }

    void set(float vals[16]) {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                m[r][c] = clamp_analog(vals[r * 4 + c]);
    }
};

// --- Multiply Two 4x4 Matrices ---

inline Matrix4x4 multiply(const Matrix4x4& a, const Matrix4x4& b) {
    Matrix4x4 result;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[r][k] * b.m[k][c];
            }
            result.m[r][c] = clamp_analog(sum);
        }
    }
    return result;
}

// --- Build Translation Matrix (3D) ---

inline Matrix4x4 translation_matrix_3d(float tx, float ty, float tz) {
    Matrix4x4 mat;
    mat.m[0][3] = clamp_analog(tx);
    mat.m[1][3] = clamp_analog(ty);
    mat.m[2][3] = clamp_analog(tz);
    return mat;
}

// --- Build Rotation Matrix Around Z Axis (3D) ---

inline Matrix4x4 rotation_matrix_z_3d(float angle_radians) {
    Matrix4x4 mat;
    float c = clamp_analog(cosf(angle_radians));
    float s = clamp_analog(sinf(angle_radians));

    mat.m[0][0] = c;
    mat.m[0][1] = -s;
    mat.m[1][0] = s;
    mat.m[1][1] = c;

    return mat;
}

// --- Build Rotation Matrix Around Y Axis (3D) ---

inline Matrix4x4 rotation_matrix_y_3d(float angle_radians) {
    Matrix4x4 mat;
    float c = clamp_analog(cosf(angle_radians));
    float s = clamp_analog(sinf(angle_radians));

    mat.m[0][0] = c;
    mat.m[0][2] = s;
    mat.m[2][0] = -s;
    mat.m[2][2] = c;

    return mat;
}

// --- Build Rotation Matrix Around X Axis (3D) ---

inline Matrix4x4 rotation_matrix_x_3d(float angle_radians) {
    Matrix4x4 mat;
    float c = clamp_analog(cosf(angle_radians));
    float s = clamp_analog(sinf(angle_radians));

    mat.m[1][1] = c;
    mat.m[1][2] = -s;
    mat.m[2][1] = s;
    mat.m[2][2] = c;

    return mat;
}

// --- Apply 4x4 Transform to 3D Vector (Assumes w=1) ---

inline void apply_transform_3d(const Matrix4x4& mat, float x, float y, float z, float& x_out, float& y_out, float& z_out) {
    x_out = clamp_analog(mat.m[0][0] * x + mat.m[0][1] * y + mat.m[0][2] * z + mat.m[0][3]);
    y_out = clamp_analog(mat.m[1][0] * x + mat.m[1][1] * y + mat.m[1][2] * z + mat.m[1][3]);
    z_out = clamp_analog(mat.m[2][0] * x + mat.m[2][1] * y + mat.m[2][2] * z + mat.m[2][3]);
}

} // namespace woflang
