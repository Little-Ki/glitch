#include "lib_vmt.h"
#include "lib_memory.h"

#include <Windows.h>
#include <unordered_map>

namespace cl::vmt {

	__forceinline static size_t methodCount(uintptr_t* vmt) {
		size_t i = 0;
		while (cl::memory::isValid(vmt[i]))
			++i;
		return i;
	}

	VMT::~VMT() {
		detach();
	}

	VMT::VMT(void* base)
	{
		_state.base = reinterpret_cast<ClassRef*>(base);
		_state.installed = false;
		
		if (_state.base) {
			_state.original = _state.base->vmt;
		}
	}

	bool VMT::attach() {
		std::shared_lock<std::shared_mutex> lock(_state.mutex);

		if (_state.installed) {
			return true;
		}

		if (!_state.base) {
			return false;
		}

		const auto method_count = methodCount(_state.base->vmt);

		if (!method_count) {
			return false;
		}

		_state.table.resize(method_count + 1L);

		if (!_state.table.size()) {
			return false;
		}

		std::memcpy(
			reinterpret_cast<void*>(_state.table.data()),
			reinterpret_cast<void*>(_state.original - 1L),
			(method_count + 1L) * sizeof(uintptr_t));

		_state.base->vmt = _state.table.data() + 1L;
		_state.installed = true;
		_state.count = method_count;

		return true;
	}

	bool VMT::detach() {
		std::shared_lock<std::shared_mutex> lock(_state.mutex);

		if (!_state.installed) {
			return false;
		}

		if (!_state.original || !_state.base) {
			return false;
		}

		_state.base->vmt = _state.original;
		_state.table.clear();
		_state.installed = false;
		_state.count = 0;

		return true;
	}

	bool VMT::hook(size_t index, void* detour) {
		std::shared_lock<std::shared_mutex> lock(_state.mutex);

		if (!_state.installed || index >= _state.count)
			return false;

		_state.table[index] = reinterpret_cast<uintptr_t>(detour);

		return true;
	}


	bool VMT::hook(void* function, void* detour) {
		std::shared_lock<std::shared_mutex> lock(_state.mutex);

		if (!_state.installed) 
			return false;

		const size_t index = std::find(
			_state.table.begin(),
			_state.table.end(),
			reinterpret_cast<uintptr_t>(function)
		) - _state.table.begin();

		if (index >= _state.count) 
			return false;

		_state.table[index] = reinterpret_cast<uintptr_t>(detour);

		return true;
	}
}