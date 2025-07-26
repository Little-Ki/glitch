#pragma once

#include <Windows.h>
#include <cstdint>
#include <functional>

namespace cl::thread {
	HANDLE create(void* function, void* param, std::function<uint8_t* (size_t)> alloc = nullptr);
}