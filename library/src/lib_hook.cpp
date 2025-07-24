#include "lib_hook.h"
#include "lib_dasm.h"
#include "lib_memory.h"

#include <Windows.h>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#pragma warning(disable : 4244)

namespace cl::hook {

#ifdef _WIN64
	constexpr auto IS_WIN64 = true;
#else
	constexpr auto IS_WIN64 = false;
#endif

	static std::shared_mutex mutex;

	struct HookRecord {
		void* src;
		void* proxy;
		int stub_size;
	};

	static std::unordered_map<void*, HookRecord> records;

	bool create(void* fn, void* detour, void** proxy) {
		std::shared_lock<std::shared_mutex> lock(mutex);

		if (records.find(fn) != records.end()) {
			*proxy = records[fn].proxy;
			return true;
		}

		HookRecord record;

		uint8_t* bytes = reinterpret_cast<uint8_t*>(fn);
		uint8_t* tramp = nullptr;

		int stub_size = 0;

		while (stub_size < 5) {
			const auto len = cl::dasm::asmlen(&bytes[stub_size], IS_WIN64);
			stub_size += len;
		}

		{
			uint8_t* ptr = reinterpret_cast<uint8_t*>(bytes) - 0x2000;

			while (!tramp) {
				tramp = reinterpret_cast<uint8_t*>(
					VirtualAlloc(ptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
				ptr = ptr + 0x200;
			}
		}

		record.src = fn;
		record.proxy = tramp;
		record.stub_size = stub_size;

		memcpy_s(tramp, stub_size, bytes, stub_size);

		uint8_t* jumpers = tramp + stub_size;

#ifdef _WIN64
		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // address of detour function
			};

			int* rel = reinterpret_cast<int*>(&jmp[1]);
			uintptr_t* abs = reinterpret_cast<uintptr_t*>(&jmp[11]);

			*rel = (bytes + stub_size) - jumpers - 5;
			*abs = reinterpret_cast<uintptr_t>(detour);

			if (!cl::memory::write(jumpers, jmp, sizeof(jmp)))
				return false;
		}
#else
		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0xE9, 0x00, 0x00, 0x00, 0x00 };

			int* rel0 = reinterpret_cast<int*>(&jmp[1]);
			int* rel1 = reinterpret_cast<int*>(&jmp[6]);

			*rel0 = static_cast<int>(bytes + stub_size - jumpers) - 5;
			*rel1 = static_cast<int>(reinterpret_cast<uint8_t*>(detour) - (jumpers + 5)) - 5;

			if (!cl::memory::write(jumpers, jmp, sizeof(jmp)))
				return false;
		}
#endif
		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0x09, 0x09, 0x09, 0x09, 0x09,
				0x09, 0x09, 0x09, 0x09, 0x09 };

			int* rel = reinterpret_cast<int*>(&jmp[1]);
			*rel = (jumpers + 5 - bytes) - 5;

			if (!cl::memory::write(bytes, jmp, stub_size))
				return false;
		}

		*proxy = tramp;

		records[fn] = record;

		return false;
	}

	void releaseAll() {
		std::shared_lock<std::shared_mutex> lock(mutex);

		for (const auto& r : records) {
			cl::memory::write(
				reinterpret_cast<uint8_t*>(r.second.src),
				reinterpret_cast<uint8_t*>(r.second.proxy),
				r.second.stub_size);
		}

		records.clear();
	}
}