#pragma once

#include <Windows.h>

namespace cl::thread {
    HANDLE create(void* function, void* param);
}