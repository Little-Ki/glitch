#pragma once
#include "lib_matrix.h"
#include "lib_vector.h"

namespace cl::math {

    constexpr float PI = 3.1415926535f;

    __forceinline float deg_to_rad(const float &degree) {
        return degree / 180.f * PI;
    }

    __forceinline float rad_to_deg(const float &radian) {
        return (radian / PI) * 180.f;
    }

    float dot(const Vec3 &a, const Vec3 &b);

    Vec3 cross(const Vec3 &a, const Vec3 &b);

    Vec3 normalize(const Vec3 &v);

    Vec3 transform(const Vec3 &v, const Mat4 &m);
    
    Vec3 transform(const Vec3 &v, const Mat3 &m);

    Mat4 lookAtLH(const Vec3 &eye, const Vec3 &at, const Vec3 &up);

    Mat4 lookAtRH(const Vec3 &eye, const Vec3 &at, const Vec3 &up);

    Vec3 rotate(const Vec3 &v, const float &angle, const Vec3 &normal);
}