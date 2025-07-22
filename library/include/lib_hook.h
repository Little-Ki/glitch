#pragma once

#include <cctype>

namespace cl::hook {

    bool create(void *function, void *detour, void **proxy);

    void releaseAll();

}