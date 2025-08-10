// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <Windows.h>
#include "lib_vmt.h"
#include "lib_hash.h"
#include "lib_console.h"
#include "lib_print.h"
#include "lib_pe.h"
#include "lib_thread.h"
#include "lib_internal.h"

#include "ct_menu.h"
#include "ct_bypass.h"
#include "ct_feature.h"

static void mainThread(void* handle) {
	ct::bypass::install(handle);
	ct::menu::install();
	ct::feature::install();
	cl::pe::headless(handle);

	while (!GetAsyncKeyState(VK_END));

	ct::menu::uninstall();
	ct::bypass::uninstall();
	cl::vmt::detach();

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, handle, 0, 0);
}

static BOOL APIENTRY DllMain(
	HMODULE handle,
	DWORD reason,
	LPVOID reserved
) {
	if (reason == DLL_PROCESS_ATTACH) {

		cl::internal::DisableThreadLibraryCalls(handle);
		//cl::internal::SetWindowDisplayAffinity();
		cl::thread::create(mainThread, handle);
	}

	return TRUE;
}
