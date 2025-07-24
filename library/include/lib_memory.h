#pragma once
#include <cstdint>

namespace cl::memory {
    bool isValid(uintptr_t address);

    bool write(void *dst, void *src, size_t size);
}