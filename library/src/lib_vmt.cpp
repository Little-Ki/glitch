#include "lib_vmt.h"
#include "lib_memory.h"
#include "lib_internal.h"

#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace cl::vmt {

	static std::shared_mutex mutex;

	struct InterRecord {
		void** table;
		void* original;
		size_t index;
	};

	static std::unordered_map<void*, InterRecord> records;

	void attach(void* instance, size_t index, void* detour) {
		std::shared_lock<std::shared_mutex> lock(mutex);

		if (records.find(detour) != records.end()) {
			return;
		}

		auto table = *reinterpret_cast<void***>(instance);
		auto entry = &table[index];
		auto original = *entry;
		auto old = 0UL;

		if (!cl::internal::VirtualProtect(entry, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old)) {
			return;
		}

		*entry = detour;

		cl::internal::VirtualProtect(entry, sizeof(uintptr_t), old, &old);

		InterRecord record{ 0 };

		record.table = table;
		record.original = original;
		record.index = index;

		records[detour] = record;
	}

	void detach(void* detour) {
		std::shared_lock<std::shared_mutex> lock(mutex);

		auto it = records.find(detour);
		auto old = 0UL;

		if (it != records.end()) {
			auto& record = it->second;

			auto entry = &record.table[record.index];

			if (!cl::internal::VirtualProtect(entry, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old)) {
				return;
			}

			*entry = record.original;

			cl::internal::VirtualProtect(entry, sizeof(uintptr_t), old, &old);

			records.erase(detour);
		}
	}

	void detach() {
		std::shared_lock<std::shared_mutex> lock(mutex);

		for (const auto& r : records) {
			auto& record = r.second;
			auto entry = &record.table[record.index];
			auto old = 0UL;

			if (!cl::internal::VirtualProtect(entry, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old)) {
				continue;
			}

			*entry = record.original;

			cl::internal::VirtualProtect(entry, sizeof(uintptr_t), old, &old);
		}

		records.clear();
	}

	void* original(void* detour) {
		auto it = records.find(detour);
		return it == records.end() ? nullptr : it->second.original;
	}
}