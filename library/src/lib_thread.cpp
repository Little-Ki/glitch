#include "lib_thread.h"
#include "lib_memory.h"
#include "lib_pe.h"
#include "lib_hash.h"

#include <cstdint>
#include <memory>

namespace cl::thread {

	static uint8_t* defaultAlloc(size_t size) {
		return reinterpret_cast<uint8_t*>(
			VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
		);
	}

	HANDLE create(void* function, void* param, std::function<uint8_t* (size_t)> alloc) {

		void* enter = nullptr;

#ifdef _WIN64
		uint8_t jmp[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		enter = alloc ? alloc(14) : defaultAlloc(14);

		if (!enter) return nullptr;

		auto dst = reinterpret_cast<uintptr_t*>(&jmp[6]);

		*dst = reinterpret_cast<uintptr_t>(function);

#else
		uint8_t jmp[] = {
			0xE9, 0x00, 0x00, 0x00, 0x00,
		};

		enter = alloc ? alloc(5) : defaultAlloc(5);

		if (!enter) return nullptr;

		auto rel = reinterpret_cast<int*>(&jmp[1]);

		*rel = static_cast<int>(
			reinterpret_cast<uint8_t*>(function) -
			reinterpret_cast<uint8_t*>(enter) - 5
			);
#endif

		if (!cl::memory::write(enter, jmp, sizeof(jmp)))
			return nullptr;

		return CreateThread(0, 0, (LPTHREAD_START_ROUTINE)enter, param, 0, 0);
	}
}