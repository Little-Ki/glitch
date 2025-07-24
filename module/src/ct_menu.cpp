#include "ct_menu.h"

#include <Windows.h>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ct::menu {
	
	static WNDPROC oProcess = nullptr;

	static HWND window = nullptr;

	void render()
	{
		ImGui::SetNextWindowPos({ 100, 100 });
		ImGui::Begin("Hello");

		ImGui::Text("Hello");
		ImGui::Button("World!");

		ImGui::End();
	}

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

		return ::CallWindowProcA(oProcess, hwnd, uMsg, wParam, lParam);
	}

	
	void watch(void* hwnd) {
		if (!window) {
			window = reinterpret_cast<HWND>(hwnd);

			oProcess = reinterpret_cast<WNDPROC>(
				::SetWindowLongPtr(
					window,
					GWLP_WNDPROC, 
					reinterpret_cast<uintptr_t>(hkWindowProc))
				);
		}
	}

	void unwatch() {
		if (window) {
			SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<uintptr_t>(oProcess));
			window = nullptr;
		}
	}

}