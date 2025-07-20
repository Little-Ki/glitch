#include "lib_pe.h"
#include "lib_nt.h"

#include <algorithm>

using namespace cl::hash;

static __forceinline TEB *GetTEB() {
#ifdef _WIN64
    return reinterpret_cast<TEB *>(__readgsqword(0x30));
#else
    return reinterpret_cast<TEB *>(__readfsdword(0x18));
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

            // std::wstring mod_name(entry->BaseDllName.szBuffer);

            if (hash::fnv1a(entry->BaseDllName.szBuffer) == name) {
                return { 
                    reinterpret_cast<uintptr_t>(entry->DllBase),
                    entry->SizeOfImage  
                };
            }
        }

        return { 0 };
    }

    uintptr_t getExport(const Module& mod, const hash_t& proc)
    {
        auto dos            = reinterpret_cast<PIMAGE_DOS_HEADER>(mod.base);
        auto nt             = reinterpret_cast<PIMAGE_NT_HEADERS>(mod.base + dos->e_lfanew);
        auto export_entry   = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        if (!export_entry.Size) return 0;

        auto export_dir     = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(mod.base + export_entry.VirtualAddress);

        auto* addr_table    = reinterpret_cast<uintptr_t*>(mod.base + export_dir->AddressOfFunctions);
        auto* name_table    = reinterpret_cast<uint32_t*>(mod.base + export_dir->AddressOfNames);
        auto* index_table   = reinterpret_cast<uint16_t*>(mod.base + export_dir->AddressOfNameOrdinals);

        for (uint16_t i = 0; i < export_dir->NumberOfNames; i++) {
            const auto name = reinterpret_cast<char*>(mod.base + name_table[i]);
            const auto index = index_table[i];

            if (hash::fnv1a(name) != proc) {
                continue;
            }

            return mod.base + addr_table[index];
        }

        return 0;
    }
}