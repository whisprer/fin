#pragma once

#include "analog_modes.hpp"
#include <cmath>

namespace woflang {

// --- 2x2 Matrix Struct ---

struct Matrix2x2 {
    float m00, m01;
    float m10, m11;

    Matrix2x2() : m00(1.0f), m01(0.0f), m10(0.0f), m11(1.0f) {}  // Identity by default

    void set(float a, float b, float c, float d) {
        m00 = clamp_analog(a);
        m01 = clamp_analog(b);
        m10 = clamp_analog(c);
        m11 = clamp_analog(d);
    }
};

// --- 3x3 Matrix Struct ---

struct Matrix3x3 {
    float m00, m01, m02;
    float m10, m11, m12;
    float m20, m21, m22;

    Matrix3x3() : 
        m00(1.0f), m01(0.0f), m02(0.0f),
        m10(0.0f), m11(1.0f), m12(0.0f),
        m20(0.0f), m21(0.0f), m22(1.0f) {}  // Identity by default

    void set(
        float a, float b, float c,
        float d, float e, float f,
        float g, float h, float i
    ) {
        m00 = clamp_analog(a); m01 = clamp_analog(b); m02 = clamp_analog(c);
        m10 = clamp_analog(d); m11 = clamp_analog(e); m12 = clamp_analog(f);
        m20 = clamp_analog(g); m21 = clamp_analog(h); m22 = clamp_analog(i);
    }
};

// --- Matrix Multiply 2x2 ---

inline Matrix2x2 multiply(const Matrix2x2& a, const Matrix2x2& b) {
    Matrix2x2 result;
    result.m00 = clamp_analog(a.m00 * b.m00 + a.m01 * b.m10);
    result.m01 = clamp_analog(a.m00 * b.m01 + a.m01 * b.m11);
    result.m10 = clamp_analog(a.m10 * b.m00 + a.m11 * b.m10);
    result.m11 = clamp_analog(a.m10 * b.m01 + a.m11 * b.m11);
    return result;
}

// --- Matrix Multiply 3x3 ---

inline Matrix3x3 multiply(const Matrix3x3& a, const Matrix3x3& b) {
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

} // namespace woflang
