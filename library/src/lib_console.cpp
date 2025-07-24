#include "lib_console.h"
#include <Windows.h>

namespace cl::console {
    bool allocate(const std::string &title) {
        if (!AllocConsole()) {
            return false;
        }

        _iobuf *data;
        const errno_t res = freopen_s(&data, "CONOUT$", "w", stdout);
        if (res != 0) {
            return false;
        }

        if (!SetConsoleTitleA(title.c_str())) {
            return false;
        }

        return true;
    }

    void detach() {
        FreeConsole();
    }
}