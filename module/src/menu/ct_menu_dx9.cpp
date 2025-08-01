#include "ct_menu.h"

#define RELEASE(p)    \
    if (p) {          \
        p->Release(); \
        p = nullptr;  \
    }

#include <Windows.h>
#include <d3d9.h>
#include <dxgi.h>
#include <iostream>

#include "lib_hook.h"

#pragma comment(lib, "d3d19.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

namespace ct::menu {

    using ResetFn = HRESULT(__stdcall *)(IDirect3DDevice9 *, D3DPRESENT_PARAMETERS *);
    using EndSceneFn = HRESULT(__stdcall *)(IDirect3DDevice9 *);

    static ResetFn oReset = nullptr;
    static EndSceneFn oEndScene = nullptr;

    static HRESULT __stdcall hkReset(IDirect3DDevice9 *self, D3DPRESENT_PARAMETERS *params) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
        long result = oReset(self, params);
        ImGui_ImplDX9_CreateDeviceObjects();

        return result;
    }

    static HRESULT __stdcall hkEndScene(IDirect3DDevice9 *self) {
        static bool init = false;
        static bool show_demo = true;

        if (!init) {
            D3DDEVICE_CREATION_PARAMETERS params;
            self->GetCreationParameters(&params);
            ImGui_ImplWin32_EnableDpiAwareness();

            menu::watch(params.hFocusWindow);

            ImGui::CreateContext();

            auto &io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;

            ImGui_ImplWin32_Init(params.hFocusWindow);
            ImGui_ImplDX9_Init(self);

            init = true;
        } else {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow(&show_demo);

            // menu::render();

            ImGui::EndFrame();
            ImGui::Render();

            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }

        return oEndScene(self);
    }

    bool install() {
        D3DPRESENT_PARAMETERS params; 
        LPDIRECT3D9 dx9;
        LPDIRECT3DDEVICE9 device = nullptr;

        if ((dx9 = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) {
            return false;
        }

        params.BackBufferWidth = 0;
        params.BackBufferHeight = 0;
        params.BackBufferFormat = D3DFMT_UNKNOWN;
        params.BackBufferCount = 0;
        params.MultiSampleType = D3DMULTISAMPLE_NONE;
        params.MultiSampleQuality = NULL;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.hDeviceWindow = GetForegroundWindow();
        params.Windowed = 1;
        params.EnableAutoDepthStencil = 0;
        params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
        params.Flags = NULL;
        params.FullScreen_RefreshRateInHz = 0;
        params.PresentationInterval = 0;

        if (dx9->CreateDevice(
                D3DADAPTER_DEFAULT,
                D3DDEVTYPE_NULLREF,
                GetForegroundWindow(),
                D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
                &params, &device) < 0) {
            dx9->Release();
            return false;
        }

        void **vmt = *(void ***)device;

        RELEASE(device);
        RELEASE(dx9);

        return cl::hook::trampoline(vmt[16], hkReset, (void **)(&oReset)) &&
               cl::hook::trampoline(vmt[42], hkEndScene, (void **)(&oEndScene));
    }

    void uninstall() {
        menu::unwatch();
    }

}