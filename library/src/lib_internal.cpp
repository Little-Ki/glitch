#include "lib_internal.h"

#include "lib_pe.h"
#include "lib_hash.h"

namespace cl::internal {

	SIZE_T VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
	{
		static const auto t = pe::getExport<SIZE_T(__stdcall*)(LPCVOID, void*, SIZE_T)>(
			CT_HASH("kernel32.dll"), CT_HASH("VirtualQuery")
		);
		return t ? t(lpAddress, lpBuffer, dwLength) : SIZE_T();
	}

	LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
	{
		static const auto t = pe::getExport<LPVOID(__stdcall*)(LPVOID, SIZE_T, DWORD, DWORD)>(
			CT_HASH("kernel32.dll"), CT_HASH("VirtualAlloc")
		);
		return t ? t(lpAddress, dwSize, flAllocationType, flProtect) : LPVOID();
	}

	BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
	{
		static const auto t = pe::getExport<BOOL(__stdcall*)(LPVOID, SIZE_T, DWORD)>(
			CT_HASH("kernel32.dll"), CT_HASH("VirtualFree")
		);
		return t ? t(lpAddress, dwSize, dwFreeType) : BOOL();
	}

	BOOL VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
	{
		static const auto t = pe::getExport<BOOL(__stdcall*)(LPVOID, SIZE_T, DWORD, PDWORD)>(
			CT_HASH("kernel32.dll"), CT_HASH("VirtualProtect")
		);
		return t ? t(lpAddress, dwSize, flNewProtect, lpflOldProtect) : BOOL();
	}

	HANDLE CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
	{
		static const auto t = pe::getExport<HANDLE(__stdcall*)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD)>(
			CT_HASH("kernel32.dll"), CT_HASH("CreateThread")
		);
		return t ? t(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId) : HANDLE();
	}

	BOOL AllocConsole()
	{
		static const auto t = pe::getExport<BOOL(__stdcall*)()>(
			CT_HASH("kernel32.dll"), CT_HASH("AllocConsole")
		);
		return t ? t() : BOOL();
	}

	BOOL SetConsoleTitleA(LPCTSTR lpConsoleTitle)
	{
		static const auto t = pe::getExport<BOOL(__stdcall*)(LPCTSTR)>(
			CT_HASH("kernel32.dll"), CT_HASH("SetConsoleTitleA")
		);
		return t ? t(lpConsoleTitle) : BOOL();
	}
	
	BOOL FreeConsole()
	{
		static const auto t = pe::getExport<BOOL(__stdcall*)()>(
			CT_HASH("kernel32.dll"), CT_HASH("FreeConsole")
		);
		return t ? t() : BOOL();
	}

	BOOL DisableThreadLibraryCalls(HMODULE hLibModule)
	{
		static const auto t = pe::getExport<BOOL(__stdcall*)(HMODULE)>(
			CT_HASH("kernel32.dll"), CT_HASH("DisableThreadLibraryCalls")
		);
		return t ? t(hLibModule) : BOOL();
	}
}