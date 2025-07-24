// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <Windows.h>
#include "lib_hook.h"
#include "lib_console.h"
#include "lib_print.h"
#include "lib_pe.h"
#include "ct_menu.h"
#include "ct_bypass.h"

static void MainThread(void* handle) {
	ct::menu::install();
	ct::bypass::install();
	cl::pe::headless(handle);

	while (!GetAsyncKeyState(VK_END));

	ct::menu::uninstall();
	ct::bypass::uninstall();
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
