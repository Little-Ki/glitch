#pragma once
#include <cassert>
#include <stdint.h>

#include "lib_vector.h"

namespace cl::math {
    class Mat3 {
        Vec3 raw[3];

    public:
        Vec3 &operator[](uint8_t index) {
            assert(index >= 0 && index < 3);
            return raw[index];
        }

        const Vec3 &operator[](uint8_t index) const {
            assert(index >= 0 && index < 3);
            return raw[index];
        }
    };

    class Mat4 {
        Vec4 raw[4];

    public:
        Vec4 &operator[](uint8_t index) {
            assert(index >= 0 && index < 4);
            return raw[index];
        }

        const Vec4 &operator[](uint8_t index) const {
            assert(index >= 0 && index < 4);
            return raw[index];
        }
    };
}