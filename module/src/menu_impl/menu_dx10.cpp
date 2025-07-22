#include "menu.h"

#define RELEASE(p)    \
    if (p) {          \
        p->Release(); \
        p = nullptr;  \
    }

#include <Windows.h>
#include <d3d10.h>
#include <dxgi.h>
#include <iostream>

#include "lib_hook.h"

#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "imgui.h"
#include "imgui_impl_dx10.h"
#include "imgui_impl_win32.h"

namespace ct::menu {

    using PresentFn = HRESULT(__stdcall *)(IDXGISwapChain *, UINT, UINT);

    static PresentFn oPresent = nullptr;

    static HRESULT __stdcall hkPresent(IDXGISwapChain *self, UINT sync_interval, UINT flags) {

        static bool init = false;
        static bool show_demo = true;

        static ID3D10RenderTargetView *rt_view{nullptr};

        DXGI_SWAP_CHAIN_DESC desc;

        ID3D10Device *device{nullptr};

        ID3D10Texture2D *back_buffer{nullptr};

        self->GetDesc(&desc);
        self->GetDevice(__uuidof(ID3D10Device), (void **)&device);

        if (!init) {
            ImGui_ImplWin32_EnableDpiAwareness();

            menu::watch(desc.OutputWindow);

            ImGui::CreateContext();

            auto &io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;

            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX10_Init(device);

            init = true;
        } else {

            self->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

            if (back_buffer) {
                device->CreateRenderTargetView(back_buffer, nullptr, &rt_view);
                back_buffer->Release();

                ImGui_ImplDX10_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                ImGui::ShowDemoWindow(&show_demo);

                // menu::render();

                ImGui::EndFrame();
                ImGui::Render();

                device->OMSetRenderTargets(1, &rt_view, nullptr);

                ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());

                RELEASE(rt_view);
            }
        }

        return tPresent(self, sync_interval, flags);
    }

    bool install() {
        ID3D10Device *device = nullptr;
        IDXGISwapChain *swapchain = nullptr;
        D3D_FEATURE_LEVEL featureLevel;
        DXGI_SWAP_CHAIN_DESC sd{0};

        sd.BufferCount = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.Height = 800;
        sd.BufferDesc.Width = 600;
        sd.BufferDesc.RefreshRate = {60, 1};
        sd.OutputWindow = GetForegroundWindow();
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;

        HRESULT hr = D3D10CreateDeviceAndSwapChain(
            nullptr,
            D3D10_DRIVER_TYPE_HARDWARE,
            nullptr, 0, D3D10_SDK_VERSION,
            &sd, &swapchain, &device);

        if (FAILED(hr)) {
            return false;
        }

        void **vmt = *(void ***)swapchain;

        RELEASE(swapchain);
        RELEASE(device);

        return cl::hook::create(vmt[8], hkPresent, (void **)(&oPresent));
    }

    void uninstall() {
        menu::unwatch();
    }

}