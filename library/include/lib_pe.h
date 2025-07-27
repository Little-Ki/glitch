#pragma once
#include "lib_hash.h"
#include <string>

namespace cl::pe {
    bool getModule(const hash_t& name, void** base = nullptr, size_t* size = nullptr);

    void* getExport(const hash_t& name, const hash_t& proc);

    template<typename T>
    T getExport(const hash_t& name, const hash_t& proc) {
        return reinterpret_cast<T>(getExport(name, proc));
    };

    void* findCave(const hash_t& name, size_t size);

    bool headless(void* handle);
}