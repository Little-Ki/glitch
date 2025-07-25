#pragma once

#include <cstdint>
#include <functional>

namespace cl::hook {

	struct TrampSize {
		size_t total_size;
		size_t stub_size;
		size_t jump_size;
	};

	uint8_t* defaultAlloc(void* entry, size_t require);

	bool trampoline(void* entry, void* detour, void** tramp, std::function<uint8_t*(void*, size_t)> allocator = defaultAlloc);

	TrampSize measureSize(void* entry);

	void releaseAll();
}