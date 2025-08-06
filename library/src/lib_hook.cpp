#include "lib_hook.h"
#include "lib_dasm.h"
#include "lib_memory.h"
#include "lib_internal.h"

#include <Windows.h>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#pragma warning(disable : 4244)

#ifdef _WIN64
constexpr bool IsWin64 = true;
#else
constexpr bool IsWin64 = false;
#endif

namespace cl::hook {

	static std::shared_mutex mutex;

	struct HookRecord {
		void* entry;
		void* tramp;
		size_t stub_size;
	};

	// mapping detour -> record
	static std::unordered_map<void*, HookRecord> records;

	static uint8_t* defaultAlloc(void* entry, size_t require)
	{
		uint8_t* result = nullptr;
		uint8_t* ptr = reinterpret_cast<uint8_t*>(entry) - 0x2000;

		while (!result) {
			result = reinterpret_cast<uint8_t*>(
				internal::VirtualAlloc(
					ptr, require, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE
				));
			ptr += 0x200;
		}

		return result;
	}

	void attach(void* _entry, void* detour, std::function<uint8_t* (void*, size_t)> alloc)
	{
		std::shared_lock<std::shared_mutex> lock(mutex);

		if (records.find(detour) != records.end()) {
			return;
		}

		uint8_t* entry = reinterpret_cast<uint8_t*>(_entry);
		uint8_t* tramp = nullptr;

		size_t stub_size = 0;

		while (stub_size < 5) {
			const auto len = cl::dasm::asmlen(&entry[stub_size], IsWin64);
			stub_size += len;
		}

#ifdef _WIN64
		uint8_t jmp[] = {
			0xE9, 0x00, 0x00, 0x00, 0x00,
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
#else
		uint8_t jmp[] = {
			0xE9, 0x00, 0x00, 0x00, 0x00,
			0xE9, 0x00, 0x00, 0x00, 0x00
		};
#endif

		auto require_size = stub_size + sizeof(jmp);

		tramp = alloc ? alloc(entry, require_size) : defaultAlloc(entry, require_size);

		if (!tramp) return;

		if (!cl::memory::write(tramp, entry, stub_size))
			return;

		uint8_t* jmps = tramp + stub_size;

#ifdef _WIN64
		{
			int* rel = reinterpret_cast<int*>(&jmp[1]);
			uintptr_t* abs = reinterpret_cast<uintptr_t*>(&jmp[11]);

			*rel = (entry + stub_size) - jmps - 5;
			*abs = reinterpret_cast<uintptr_t>(detour);

			if (!cl::memory::write(jmps, jmp, sizeof(jmp)))
				return;
		}
#else
		{
			int* rel0 = reinterpret_cast<int*>(&jmp[1]);
			int* rel1 = reinterpret_cast<int*>(&jmp[6]);

			*rel0 = static_cast<int>(entry + stub_size - jmps) - 5;
			*rel1 = static_cast<int>(reinterpret_cast<uint8_t*>(detour) - (jmps + 5)) - 5;

			if (!cl::memory::write(jmps, jmp, sizeof(jmp)))
				return;
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
				return;
		}

		HookRecord record{ 0 };

		record.entry = entry;
		record.tramp = tramp;
		record.stub_size = stub_size;

		records[detour] = record;
	}

	void detach() {
		std::shared_lock<std::shared_mutex> lock(mutex);

		for (const auto& r : records) {
			cl::memory::write(
				reinterpret_cast<uint8_t*>(r.second.entry),
				reinterpret_cast<uint8_t*>(r.second.tramp),
				r.second.stub_size);
		}

		records.clear();
	}

	void detach(void* detour)
	{
		std::shared_lock<std::shared_mutex> lock(mutex);

		auto it = records.find(detour);

		if (it != records.end()) {
			cl::memory::write(
				reinterpret_cast<uint8_t*>(it->second.entry),
				reinterpret_cast<uint8_t*>(it->second.tramp),
				it->second.stub_size);
			records.erase(detour);
		}
	}

	const void* trampoline(void* detour)
	{
		auto it = records.find(detour);
		return it == records.end() ? nullptr : it->second.tramp;
	}
}