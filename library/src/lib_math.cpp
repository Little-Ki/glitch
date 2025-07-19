#include "lib_math.h"

namespace cl::math {

    float dot(const Vec3 &a, const Vec3 &b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Vec3 cross(const Vec3 &a, const Vec3 &b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
    }

    Vec3 normalize(const Vec3 &v) {
        const auto r = 1.0f / std::sqrtf(dot(v, v));
        return v * r;
    }

    Vec3 transform(const Vec3 &v, const Mat4 &m) {
        Vec3 result = {
            dot(v, m[0]),
            dot(v, m[1]),
            dot(v, m[2])};

        result += m[3];

        return result;
    }

    Vec3 transform(const Vec3 &v, const Mat3 &m) {
        return {
            dot(v, m[0]),
            dot(v, m[1]),
            dot(v, m[2])};
    }

    Mat4 lookAtLH(const Vec3 &eye, const Vec3 &at, const Vec3 &up) {
        const Vec3 f = normalize(at - eye);
        const Vec3 s = normalize(cross(up, f));
        const Vec3 u = cross(f, s);

        Mat4 result;
        result[0] = {s.x, u.x, -f.x, 0.0};
        result[1] = {s.y, u.y, -f.y, 0.0};
        result[2] = {s.z, u.z, -f.z, 0.0};
        result[3] = {-dot(s, eye), -dot(u, eye), dot(f, eye), 1.0};
        return result;
    }

    Mat4 lookAtRH(const Vec3 &eye, const Vec3 &at, const Vec3 &up) {
        const Vec3 f = normalize(at - eye);
        const Vec3 s = normalize(cross(up, f));
        const Vec3 u = cross(f, s);

        Mat4 result;
        result[0] = {s.x, u.x, f.x, 0.0};
        result[1] = {s.y, u.y, f.y, 0.0};
        result[2] = {s.z, u.z, f.z, 0.0};
        result[3] = {-dot(s, eye), -dot(u, eye), -dot(f, eye), 1.0};
        return Mat4();
    }

    Vec3 rotate(const Vec3 &v, const float &angle, const Vec3 &normal) {
        const Vec3 s = normalize(cross(v, normal));

        const auto sin = std::sinf(angle);
        const auto cos = std::cosf(angle);

        return s * sin + v * cos;
    }
}