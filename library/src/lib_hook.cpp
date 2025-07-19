#include "lib_hook.h"
#include "lib_dasm.h"

#include <Windows.h>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace cl::hook {

#ifdef _WIN64
	constexpr auto IS_WIN64 = true;
#else
	constexpr auto IS_WIN64 = false;
#endif


	struct HookRecord {
		void* original;
		void* trampoline;

		size_t stub_size;
	};

	static std::unordered_map<void*, HookRecord> hooking;

	static bool writeMem(uint8_t* dst, uint8_t* src, size_t size) {
		DWORD old;

		if (!VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &old))
			return false;

		memcpy(dst, src, size);

		VirtualProtect(dst, size, old, &old);

		return true;
	}

	bool create(void* fn, void* detour, void** proxy)
	{
		if (hooking.find(fn) != hooking.end()) {
			*proxy = hooking[fn].trampoline;
			return true;
		}

		HookRecord record;

		uint8_t* bytes = reinterpret_cast<uint8_t*>(fn);
		uint8_t* tramp = nullptr;

		size_t stub_size = 0;

		while (stub_size < 5) {
			const auto len = cl::dasm::asmlen(&bytes[stub_size], IS_WIN64);
			stub_size += len;
		}

		{
			uint8_t* ptr = reinterpret_cast<uint8_t*>(bytes) - 0x2000;

			while (!tramp) {
				tramp = reinterpret_cast<uint8_t*>(
					VirtualAlloc(ptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)
					);
				ptr = ptr + 0x200;
			}
		}

		record.original = fn;
		record.trampoline = tramp;
		record.stub_size = stub_size;

		memcpy_s(tramp, stub_size, bytes, stub_size);

		uint8_t* jumpers = tramp + stub_size;

#ifdef  _WIN64
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

			if (!writeMem(jumpers, jmp, sizeof(jmp)))
				return false;
		}
#else
		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0xE9, 0x00, 0x00, 0x00, 0x00
			};

			int* rel0 = reinterpret_cast<int*>(&jmp[1]);
			int* rel1 = reinterpret_cast<int*>(&jmp[6]);

			*rel0 = static_cast<int>(fn_bytes + stub_size - jumpers) - 5;
			*rel1 = static_cast<int>(detour - (jumpers + 5)) - 5;

			if (!writeMem(jumpers, jmp, sizeof(jmp)))
				return false;
		}
#endif 
		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0x09, 0x09, 0x09, 0x09, 0x09,
				0x09, 0x09, 0x09, 0x09, 0x09
			};

			int* rel = reinterpret_cast<int*>(&jmp[1]);
			*rel = (jumpers + 5 - bytes) - 5;

			if (!writeMem(bytes, jmp, stub_size))
				return false;
		}

		*proxy = tramp;

		hooking[fn] = record;

		return false;
	}

	void release(void* fn)
	{
		if (hooking.find(fn) == hooking.end()) {
			return;
		}

		const auto& record = hooking[fn];

		if (writeMem(
			reinterpret_cast<uint8_t*>(record.original), 
			reinterpret_cast<uint8_t*>(record.trampoline),
			record.stub_size
		)) {
			hooking.erase(fn);
		}
	}
}