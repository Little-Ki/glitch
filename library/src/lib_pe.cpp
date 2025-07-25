#include "lib_pe.h"
#include "lib_nt.h"
#include "lib_memory.h"

#include <algorithm>

using namespace cl::hash;

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

	Module getModule(const hash_t& name) {
		auto peb = GetPEB();

		if (!peb) {
			return { 0 };
		}

		auto ldrd = peb->LoaderData;
		auto mods = &ldrd->InMemoryOrderModuleList;
		auto node = mods->Flink;

		while (node != mods) {
			auto entry = reinterpret_cast<LDR_DATA_TABLE_ENTRY*>
				CONTAINING_RECORD(
					node,
					LDR_DATA_TABLE_ENTRY,
					InMemoryOrderLinks);

			if (!entry)
				continue;

			if (hash::fnv1a(entry->BaseDllName.szBuffer) == name) {
				return {
					reinterpret_cast<uintptr_t>(entry->DllBase),
					entry->SizeOfImage
				};
			}
		}

		return { 0 };
	}

	void* getExport(const Module& mod, const hash_t& proc)
	{
		auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(mod.base);
		auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(mod.base + dos->e_lfanew);
		auto export_entry = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

		if (!export_entry.Size) return nullptr;

		auto export_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(mod.base + export_entry.VirtualAddress);

		auto* addr_table = reinterpret_cast<uintptr_t*>(mod.base + export_dir->AddressOfFunctions);
		auto* name_table = reinterpret_cast<uint32_t*>(mod.base + export_dir->AddressOfNames);
		auto* index_table = reinterpret_cast<uint16_t*>(mod.base + export_dir->AddressOfNameOrdinals);

		for (uint16_t i = 0; i < export_dir->NumberOfNames; i++) {
			const auto name = reinterpret_cast<char*>(mod.base + name_table[i]);
			const auto index = index_table[i];

			if (hash::fnv1a(name) != proc) {
				continue;
			}

			return reinterpret_cast<void*>( mod.base + addr_table[index]);
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

	void* findCave(void* handle, size_t size) {
		auto base = reinterpret_cast<uint8_t*>(handle);

		auto check = [](void* base, size_t size) {
			const auto p = reinterpret_cast<uint8_t*>(base);

			for (auto i = 0; i < (size + 1) / 2; i++) {
				if (!cl::memory::isValid(p + i)) return false;
				if (!cl::memory::isValid(p + size - 1 - i)) return false;
				if (p[i] != 0x90) return false;
				if (p[size - 1 - i]) return false;
			}

			return true;
			};

		if (size == 0) return nullptr;

		if (!cl::memory::isValid(handle)) return nullptr;

		auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);

		if (dos->e_magic != 'MZ') return 0;

		auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);

		auto sections = reinterpret_cast<PIMAGE_SECTION_HEADER>(base + dos->e_lfanew + sizeof(IMAGE_NT_HEADERS));

		for (auto i = 0; i < nt->FileHeader.NumberOfSections; i++) {
			auto& section = sections[i];

			if (!(section.Characteristics & IMAGE_SCN_CNT_CODE)) continue;

			auto raw = reinterpret_cast<uint8_t*>(base + section.VirtualAddress);

			for (auto j = 0; j < section.SizeOfRawData - size; j++) {
				const auto p = base + section.VirtualAddress + j - size;

				if (check(p, size)) {
					return p;
				}
			}

		}

		return nullptr;
	}
}