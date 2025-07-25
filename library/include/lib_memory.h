#pragma once
#include <cstdint>

namespace cl::memory {
    bool isValid(void* address);

    bool write(void *dst, void *src, size_t size);
}