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
	constexpr bool IsWin64 = true;
#else
	constexpr bool IsWin64 = false;
#endif

	static std::shared_mutex mutex;

	struct HookRecord {
		void* src;
		void* tramp;
		int stub_size;
	};

	static std::unordered_map<void*, HookRecord> records;
	TrampSize measureSize(void* entry)
	{
		TrampSize result{ 0 };
		auto bytes = reinterpret_cast<uint8_t*>(entry);

		result.jump_size = IsWin64 ? 19 : 10;

		while (result.stub_size < 5) {
			const auto len = cl::dasm::asmlen(&bytes[result.stub_size], IsWin64);
			result.stub_size += len;
		}

		result.total_size = result.stub_size + result.jump_size;

		return result;
	}

	uint8_t* defaultAlloc(void* entry, size_t require)
	{
		uint8_t* result = nullptr;

		{
			uint8_t* ptr = reinterpret_cast<uint8_t*>(entry) - 0x2000;

			while (!result) {
				result = reinterpret_cast<uint8_t*>(
					VirtualAlloc(ptr, require, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
				ptr = ptr + 0x200;
			}
		}

		return result;
	}

	bool trampoline(void* _entry, void* detour, void** _tramp, std::function<uint8_t* (void*, size_t)> allocator)
	{
		std::shared_lock<std::shared_mutex> lock(mutex);

		if (records.find(_entry) != records.end()) {
			*_tramp = records[_entry].tramp;
			return true;
		}

		HookRecord record{ 0 };

		uint8_t* entry = reinterpret_cast<uint8_t*>(_entry);
		uint8_t* tramp = nullptr;

		auto measure = measureSize(entry);
		auto stub_size = measure.stub_size;
		auto total_size = measure.total_size;

		tramp = allocator(entry, total_size);

		if (!tramp) {
			return false;
		}

		record.src = entry;
		record.tramp = tramp;
		record.stub_size = measure.stub_size;

		memcpy_s(tramp, stub_size, entry, stub_size);

		uint8_t* jmps = tramp + stub_size;

#ifdef _WIN64
		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // address of detour function
			};

			int* rel = reinterpret_cast<int*>(&jmp[1]);
			uintptr_t* abs = reinterpret_cast<uintptr_t*>(&jmp[11]);

			*rel = (entry + stub_size) - jmps - 5;
			*abs = reinterpret_cast<uintptr_t>(detour);

			if (!cl::memory::write(jmps, jmp, sizeof(jmp)))
				return false;
		}
#else
		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0xE9, 0x00, 0x00, 0x00, 0x00 };

			int* rel0 = reinterpret_cast<int*>(&jmp[1]);
			int* rel1 = reinterpret_cast<int*>(&jmp[6]);

			*rel0 = static_cast<int>(entry + stub_size - jmps) - 5;
			*rel1 = static_cast<int>(reinterpret_cast<uint8_t*>(detour) - (jmps + 5)) - 5;

			if (!cl::memory::write(jmps, jmp, sizeof(jmp)))
				return false;
		}
#endif

		{
			uint8_t jmp[] = {
				0xE9, 0x00, 0x00, 0x00, 0x00,
				0x90, 0x90, 0x90, 0x90, 0x90,
				0x90, 0x90, 0x90, 0x90, 0x90 };

			int* rel = reinterpret_cast<int*>(&jmp[1]);
			*rel = (jmps + 5 - entry) - 5;

			if (!cl::memory::write(entry, jmp, stub_size))
				return false;
		}

		records[_entry] = record;

		*_tramp = tramp;

		return true;
	}

	void releaseAll() {
		std::shared_lock<std::shared_mutex> lock(mutex);

		for (const auto& r : records) {
			cl::memory::write(
				reinterpret_cast<uint8_t*>(r.second.src),
				reinterpret_cast<uint8_t*>(r.second.tramp),
				r.second.stub_size);
		}

		records.clear();
	}
}