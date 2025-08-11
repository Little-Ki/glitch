#pragma once

#include <Windows.h>
#include <cstdint>
#include <functional>

namespace cl::thread {
	HANDLE create(void* function, void* param, std::function<void* (size_t)> alloc = nullptr);
}