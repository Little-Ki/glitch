#include "lib_thread.h"

#include <cstdint>
#include <memory>

namespace cl::thread {

	HANDLE create(void* function, void* param) {

		void* enter = nullptr;

#ifdef _WIN64
		uint8_t jmp[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		enter = VirtualAlloc(nullptr, 32, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (!enter) return nullptr;

		auto dst = reinterpret_cast<uintptr_t*>(& jmp[6]);

		*dst = reinterpret_cast<uintptr_t>(function);

		std::memcpy(enter, jmp, sizeof(jmp));
#else

		uint8_t jmp[] = {
			0xE9, 0x00, 0x00, 0x00, 0x00,
		};
		enter = VirtualAlloc(nullptr, 32, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (!enter) return nullptr;

		auto rel = reinterpret_cast<int*>(&jmp[1]);

		*rel = static_cast<int>(
			reinterpret_cast<uint8_t*>(function) -
			reinterpret_cast<uint8_t*>(enter) - 5
			);

		std::memcpy(enter, jmp, sizeof(jmp));
#endif

		return CreateThread(0, 0, (LPTHREAD_START_ROUTINE)enter, param, 0, 0);
	}
}