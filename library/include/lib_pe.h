#pragma once
#include "lib_hash.h"
#include <string>

namespace cl::pe {
    bool getModule(hash_t name, void** base = nullptr, size_t* size = nullptr);

    void* getExport(hash_t module_name, hash_t proc_name);

    template<typename T>
    T getExport(hash_t module_name, hash_t proc_name) {
        return reinterpret_cast<T>(getExport(name, proc));
    };

    void* findCave(hash_t module_name, size_t size);

    bool headless(void* handle);
}