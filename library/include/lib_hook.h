#pragma once

#include <cstdint>
#include <functional>

namespace cl::hook {
	bool trampoline(void* entry, void* detour, void** tramp, std::function<uint8_t*(void*, size_t)> alloc = nullptr);

	void releaseAll();
}