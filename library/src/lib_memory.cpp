#include "lib_memory.h"

#include <Windows.h>
#include <memory>

namespace cl::memory {

    bool isValid(const void* address) {
		if (!address)
			return false;

		MEMORY_BASIC_INFORMATION mbi;

		if (!VirtualQuery(address, &mbi, sizeof mbi))
			return false;

		if (mbi.Protect & PAGE_EXECUTE_READWRITE ||
			mbi.Protect & PAGE_EXECUTE_READ) {
			return true;
		}

		return false;
    }

    bool write(void *dst, void *src, size_t size) {
        DWORD old;

        if (!VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &old))
            return false;

        std::memcpy(dst, src, size);

        VirtualProtect(dst, size, old, &old);

        return true;
    }

}