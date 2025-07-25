#pragma once
#include "lib_hash.h"
#include <string>

namespace cl::pe {
    struct Module {
        uintptr_t base;
        uintptr_t size;
    };

    Module getModule(const hash_t& name);

    void* getExport(const Module& mod, const hash_t& proc);

    template<typename T>
    T getExport(const Module& mod, const hash_t& proc) {
        return reinterpret_cast<T>(getExport(mod, proc));
    };

    bool headless(void* handle);

    void* findCave(void* handle, size_t size);
}