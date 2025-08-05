// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <Windows.h>
#include "lib_hook.h"
#include "lib_hash.h"
#include "lib_console.h"
#include "lib_print.h"
#include "lib_pe.h"
#include "lib_thread.h"
#include "ct_menu.h"
#include "ct_bypass.h"
#include "ct_feature.h"

static void mainThread(void* handle) {
	ct::menu::install();
	ct::bypass::install();
	ct::feature::install();
	cl::pe::headless(handle);

	while (!GetAsyncKeyState(VK_END));

	ct::menu::uninstall();
	ct::bypass::uninstall();
	cl::hook::detach();

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, handle, 0, 0);
}

static BOOL APIENTRY DllMain(
	HMODULE handle,
	DWORD reason,
	LPVOID reserved
) {
	if (reason == DLL_PROCESS_ATTACH) {

		DisableThreadLibraryCalls(handle);
		cl::thread::create(mainThread, handle);
	}

	return TRUE;
}
