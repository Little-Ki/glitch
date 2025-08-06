#include "ct_bypass.h"
#include "lib_hook.h"

#include <cstdint>

namespace ct::bypass {

    static uintptr_t protect_base = 0;
    static uintptr_t protect_size = 0;

    void install(void* module_base) {
        // TODO: Implement anti-anti-cheat methods
    }

    void uninstall() {
        // TODO: Remove anti-anti-cheat methods
    }

}