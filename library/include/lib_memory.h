#pragma once
#include <cstdint>
#include <iterator>

namespace cl::memory {
	bool isValid(const void* address);

	bool write(void* dst, void* src, size_t size);

	template<typename T>
	class Address {
	private:
		T* ptr;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		Address(T* p = nullptr) : ptr(p) {}

		Address& operator++() { ptr += 1; return *this; }

		Address operator++(int offset) {
			Address result = *this;
			ptr += offset;
			return result;
		}

		bool operator==(Address other) const { return ptr == other.ptr; }

		bool operator!=(Address other) const { return !(*this == other); }

		T* operator*() const { return ptr; }

		T& value() { return *ptr; }
	};
}