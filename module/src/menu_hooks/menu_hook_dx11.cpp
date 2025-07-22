#include "menu_hook.h"

#define RELEASE(p) if (p) { p->Release(); p = nullptr; } 

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <iostream>

#include "lib_hook.h"
#include "menu.h"
#include "menu_win32.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

namespace ct::menu::hook {

	using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);

	static PresentFn oPresent = nullptr;

	static HRESULT __stdcall hkPresent(IDXGISwapChain* self, UINT sync_interval, UINT flags)
	{

		static bool init = false;
		static bool show_demo = true;

		static ID3D11RenderTargetView* rt_view{ nullptr };
		
		DXGI_SWAP_CHAIN_DESC desc;

		ID3D11DeviceContext* context{ nullptr };
		ID3D11Device* device{ nullptr };

		ID3D11Texture2D* back_buffer{ nullptr };

		self->GetDesc(&desc);
		self->GetDevice(__uuidof(ID3D11Device), (void**)&device);
		device->GetImmediateContext(&context);

		if (!init)
		{
			ImGui_ImplWin32_EnableDpiAwareness();

			menu::win32::install(desc.OutputWindow);

			ImGui::CreateContext();

			auto& io = ImGui::GetIO();
			io.IniFilename = nullptr;
			io.LogFilename = nullptr;

			ImGui_ImplWin32_Init(desc.OutputWindow);
			ImGui_ImplDX11_Init(device, context);

			init = true;
		}
		else {

			self->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

			if (back_buffer) {
				device->CreateRenderTargetView(back_buffer, nullptr, &rt_view);
				back_buffer->Release();

				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				ImGui::ShowDemoWindow(&show_demo);

				//menu::render();

				ImGui::EndFrame();
				ImGui::Render();

				context->OMSetRenderTargets(1, &rt_view, nullptr);

				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				RELEASE(rt_view);
			}
		}

		return oPresent(self, sync_interval, flags);
	}

	bool install() {
		ID3D11Device* device = nullptr;
		IDXGISwapChain* swapchain = nullptr;
		D3D_FEATURE_LEVEL featureLevel;
		DXGI_SWAP_CHAIN_DESC sd{ 0 };

		sd.BufferCount = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.Height = 800;
		sd.BufferDesc.Width = 600;
		sd.BufferDesc.RefreshRate = { 60, 1 };
		sd.OutputWindow = GetForegroundWindow();
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr, 0, nullptr, 0,
			D3D11_SDK_VERSION,
			&sd, &swapchain, &device, &featureLevel,
			nullptr
		);

		if (FAILED(hr)) {
			return false;
		}

		void** vmt = *(void***)swapchain;

		RELEASE(swapchain);
		RELEASE(device);

		return cl::hook::create(vmt[8], hkPresent, (void**)(&oPresent));
	}

	void uninstall() {
		menu::win32::uninstall();
	}

}