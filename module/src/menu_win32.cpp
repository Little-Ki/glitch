#include "menu_win32.h"

#include <Windows.h>
#include "backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ct::menu::win32 {

	static WNDPROC old_proc = nullptr;

	static HWND window = nullptr;

	static LRESULT CALLBACK hkWindowProc(
		_In_ HWND   hwnd,
		_In_ UINT   uMsg,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam
	)
	{
		if (hwnd == window) {

			if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) > 0)
				return 1L;
		}

		return ::CallWindowProcA(old_proc, hwnd, uMsg, wParam, lParam);
	}

	void install(HWND hwnd) {
		window = hwnd;

		old_proc = reinterpret_cast<WNDPROC>(
			::SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<uintptr_t>(hkWindowProc))
			);
	}

	void uninstall() {
		if (window) {
			SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<uintptr_t>(old_proc));
			window = nullptr;
		}
	}

}