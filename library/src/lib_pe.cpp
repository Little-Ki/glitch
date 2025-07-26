#include "lib_pe.h"
#include "lib_nt.h"
#include "lib_memory.h"

#include <algorithm>

using namespace cl::hash;

#define ALIGN(x, a) ((x + a - 1) & ~(a - 1));

static __forceinline TEB* GetTEB() {
#ifdef _WIN64
	return reinterpret_cast<TEB*>(__readgsqword(0x30));
#else
	return reinterpret_cast<TEB*>(__readfsdword(0x18));
#endif
}

static __forceinline PEB* GetPEB() {
	auto teb = GetTEB();
	return teb ? teb->Peb : nullptr;
}

namespace cl::pe {

	bool getModule(const hash_t& name, void** base, size_t* size) {
		auto peb = GetPEB();

		if (!peb) {
			return false;
		}

		auto ldr = peb->LoaderData;
		auto list = &ldr->InMemoryOrderModuleList;

		for (auto it = list->Flink; it != list; it = it->Flink) {
			auto entry = reinterpret_cast<LDR_DATA_TABLE_ENTRY*>
				CONTAINING_RECORD(
					it,
					LDR_DATA_TABLE_ENTRY,
					InMemoryOrderLinks);

			if (!entry)
				continue;

			if (hash::fnv1a(entry->BaseDllName.szBuffer) == name) {
				if (base) *base = entry->DllBase;
				if (size) *size = entry->SizeOfImage;

				return true;
			}
		}

		return false;
	}

	void* getExport(const hash_t& name, const hash_t& proc)
	{
		uint8_t* base = nullptr;

		if (!getModule(name, reinterpret_cast<void**>(&base))) {
			return nullptr;
		}

		const auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
		const auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
		const auto export_entry = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

		if (!export_entry.Size) return nullptr;

		auto export_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(base + export_entry.VirtualAddress);

		auto* addr_table = reinterpret_cast<uintptr_t*>(base + export_dir->AddressOfFunctions);
		auto* name_table = reinterpret_cast<uint32_t*>(base + export_dir->AddressOfNames);
		auto* index_table = reinterpret_cast<uint16_t*>(base + export_dir->AddressOfNameOrdinals);

		for (uint16_t i = 0; i < export_dir->NumberOfNames; i++) {
			const auto name = reinterpret_cast<char*>(base + name_table[i]);
			const auto index = index_table[i];

			if (hash::fnv1a(name) != proc) {
				continue;
			}

			return reinterpret_cast<void*>(base + addr_table[index]);
		}

		return nullptr;
	}

	bool headless(void* handle)
	{
		auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
		auto o_dos = *dos;

		std::memset(&o_dos, 0, sizeof(IMAGE_DOS_HEADER));
		o_dos.e_lfanew = dos->e_lfanew;

		return cl::memory::write(handle, &o_dos, sizeof(IMAGE_DOS_HEADER));
	}

	static bool isCave(void* base, size_t size) {
		auto beg = reinterpret_cast<uint8_t*>(base);
		auto end = reinterpret_cast<uint8_t*>(base) + size - 1;

		if (*beg != *end)
			return false;

		auto wild = *beg;

		while (beg <= end) {
			if (*beg != wild || *end != wild)
				return false;
			beg += 1;
			end -= 1;
		}

		return true;
	}

	void* findCave(const hash_t& name, size_t size) {

		uint8_t* base = nullptr;

		if (!getModule(name, reinterpret_cast<void**>(&base))) {
			return nullptr;
		}

		if (size < 5) return nullptr;

		const auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
		const auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
		const auto sections = reinterpret_cast<PIMAGE_SECTION_HEADER>(base + dos->e_lfanew + sizeof(IMAGE_NT_HEADERS));

		for (auto i = 0; i < nt->FileHeader.NumberOfSections; i++) {
			auto& section = sections[i];

			if (!(section.Characteristics & IMAGE_SCN_CNT_CODE)) continue;

			auto raw = reinterpret_cast<uint8_t*>(base + section.VirtualAddress);
			auto align_size = ALIGN(section.SizeOfRawData, 0x1000);

			for (auto j = 0; j < align_size - size; j++) {
				if (isCave(&raw[j], size)) {
					return &raw[j];
				}
			}
		}

		return nullptr;
	}

}