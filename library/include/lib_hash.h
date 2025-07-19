#pragma once

#include <string>

using hash_t = unsigned int;

namespace cl::hash {
	enum : hash_t {
		FNV1A_PRIME = 0x1000193,
		FNV1A_BASIS = 0x811C9DC5
	};

	template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
	constexpr size_t ct_strlen(const T* data) {
		size_t i = 0;
		for (; data[i] != 0; i++);
		return i;
	}

	template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
	constexpr hash_t fnv1a(const T* data, const size_t len) {
		hash_t out = FNV1A_BASIS;
		for (size_t i = 0; i < len; ++i)
			out = (out ^ data[i]) * FNV1A_PRIME;
		return out;
	}

	template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
	constexpr hash_t fnv1a(const T* data) {
		return fnv1a<T>(data, ct_strlen<T>(data));
	}

	__forceinline hash_t fnv1a(const std::string& str) {
		return fnv1a(str.c_str(), str.size());
	}

	__forceinline hash_t fnv1a(const std::wstring& str) {
		return fnv1a(str.c_str(), str.size());
	}
}

#define CT_HASH(str)                                \
    [&]() {                                         \
        constexpr hash_t result = cl::hash::fnv1a(str); \
        return result;                              \
    }()