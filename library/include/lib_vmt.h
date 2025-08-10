#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <shared_mutex>

namespace cl::vmt {

	template<typename T = void*>
	__forceinline T method(void* base, size_t index) {
		auto table = *reinterpret_cast<void ***>(base);
		return reinterpret_cast<T>(table[index]);
	}

	void attach(void* instance, size_t index, void* detour);

	void detach(void* detour);

	void detach();

	void* original(void* detour);

	template <typename T, typename... Params>
	T invoke(T(*detour)(Params...), Params... ps) {
		auto t = static_cast<decltype(detour)>(original(detour));
		return t ? t(ps...) : T();
	}
	
}