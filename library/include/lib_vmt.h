#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <shared_mutex>

namespace cl::vmt {

	class VMT {
	private:

		struct ClassRef {
			uintptr_t* vmt;
		};

		struct VmtState {
			ClassRef* base;

			size_t count{ 0 };

			uintptr_t* original{ nullptr };

			std::vector<uintptr_t> table;

			bool installed{ false };

			std::shared_mutex mutex;
		};

		VmtState _state;

	private:
		VMT() = delete;

		VMT(const VMT& other) = delete;

		VMT(const VMT&& other) = delete;

		~VMT();

	public:
		VMT(void* base);

		bool attach();

		bool detach();

		bool hook(size_t index, void* detour);

		bool hook(void* function, void* detour);

		template<typename T = uintptr_t>
		__forceinline T method(size_t index) {
			if (index > _state.count) {
				return reinterpret_cast<T>(nullptr);
			}

			if (!_state.original) {
				return reinterpret_cast<T>(nullptr);
			}

			return reinterpret_cast<T>(_state.original[index]);
		}

	};

	template<typename T = uintptr_t>
	__forceinline T method(void* base, size_t index) {
		auto table = *reinterpret_cast<void ***>(base);
		return reinterpret_cast<T>(table[index]);
	}

}