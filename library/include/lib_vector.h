#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>

namespace cl::math {

    class Vec2 {
    public:
        float x = 0.f, y = 0.f;

        Vec2(const Vec2 &o) : x(o.x), y(o.y) {};

        Vec2(float x = 0.f, float y = 0.f) : x(x), y(y) {};

        Vec2 operator+(const Vec2 &o) const {
            return {x + o.x, y + o.y};
        }

        Vec2 operator-(const Vec2 &o) const {
            return {x - o.x, y - o.y};
        }

        Vec2 operator*(const float &i) const {
            return {x * i, y * i};
        }

        Vec2 operator/(const float &i) const {
            return {x / i, y / i};
        }

        void operator+=(const Vec2 &o) {
            *this = this->operator+(o);
        }

        void operator-=(const Vec2 &o) {
            *this = this->operator-(o);
        }

        void operator*=(const float &o) {
            *this = this->operator*(o);
        }

        void operator/=(const float &o) {
            *this = this->operator/(o);
        }

        float &operator[](uint8_t index) {
            assert(index >= 0 && index < 2);
            return reinterpret_cast<float *>(&x)[index];
        }
    };

    class Vec3 : public Vec2 {
    public:
        float z = 0.f;

        Vec3(const Vec2 &v, float z = 0.f) : Vec2(v), z(z) {};

        Vec3(float x = 0.f, float y = 0.f, float z = 0.f) : Vec2(x, y), z(z) {};

        Vec3 operator+(const Vec3 &o) const {
            return {x + o.x, y + o.y, z + o.z};
        }

        Vec3 operator-(const Vec3 &o) const {
            return {x - o.x, y - o.y, z - o.z};
        }

        Vec3 operator*(const float &i) const {
            return {x * i, y * i, z * i};
        }

        Vec3 operator/(const float &i) const {
            return {x / i, y / i, z * i};
        }

        void operator+=(const Vec3 &o) {
            *this = this->operator+(o);
        }

        void operator-=(const Vec3 &o) {
            *this = this->operator-(o);
        }

        void operator*=(const float &o) {
            *this = this->operator*(o);
        }

        void operator/=(const float &o) {
            *this = this->operator/(o);
        }

        float &operator[](uint8_t index) {
            assert(index >= 0 && index < 3);
            return reinterpret_cast<float *>(&x)[index];
        }
    };

    class Vec4 : public Vec3 {
    public:
        float w = 0.f;

        Vec4(const Vec3 &v, float w = 1.f) : Vec3(v), w(w) {};

        Vec4(float x = 0.f, float y = 0.f, float z = 0.f, float w = 1.f) : Vec3(x, y, z), w(w) {};

        float &operator[](uint8_t index) {
            assert(index >= 0 && index < 4);
            return reinterpret_cast<float *>(&x)[index];
        }
    };
}