#pragma once

#include <cstdint>
#include <functional>

#include "lib_hash.h"

namespace cl::hook {

	void attach(void* entry, void* detour, std::function<uint8_t* (void*, size_t)> alloc = nullptr);

	void detach(void* detour);

	void detach();

	const void* trampoline(void* detour);

	template <typename T, typename... Params>
	T invoke(T(*detour)(Params...), Params... ps) {
		auto t = static_cast<decltype(detour)>(trampoline(detour));
		return t ? t(ps...) : T();
	}

}