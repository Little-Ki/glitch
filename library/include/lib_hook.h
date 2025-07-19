#pragma once

#include <cctype>

namespace cl::hook {

    bool create(void *src, void *detour, void **tramp);

    void release(void *src);

}