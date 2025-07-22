// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <Windows.h>
#include "lib_hook.h"
#include "lib_console.h"
#include "menu.h"
#include "bypass.h"

static void MainThread(void* handle) {
        
    cl::console::allocate("Debug");

    ct::menu::install();
    ct::bypass::install();

    while (!GetAsyncKeyState(VK_END));

    ct::menu::uninstall();
    cl::hook::releaseAll();

    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, handle, 0, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD reason,
                      LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {

        DisableThreadLibraryCalls(hModule);
        CreateThread(
            nullptr, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), 
            hModule, 
            0, nullptr
        );
    }

    return TRUE;
}
