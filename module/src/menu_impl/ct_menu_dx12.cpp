#include "ct_menu.h"

#define RELEASE(p)    \
    if (p) {          \
        p->Release(); \
        p = nullptr;  \
    }

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <vector>

#include "lib_hook.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

namespace ct::menu {

    using PresentFn = HRESULT(__stdcall *)(IDXGISwapChain3 *, UINT, UINT);
    using ExecuteCommandListFn = void(__stdcall *)(ID3D12CommandQueue *, UINT, ID3D12CommandList **);

    static PresentFn oPresent = nullptr;
    static ExecuteCommandListFn oExecCmdList = nullptr;

    struct D3D12Environment {

        struct FrameContext {
            ID3D12CommandAllocator *command_allocator;
            ID3D12Resource *resource;
            D3D12_CPU_DESCRIPTOR_HANDLE desc_handle;
        };

        ID3D12CommandQueue *command_queue = nullptr;

        ID3D12DescriptorHeap *desc_back_buffer;
        ;
        ID3D12DescriptorHeap *desc_imgui_render;
        ID3D12GraphicsCommandList *command_list;
        ID3D12CommandQueue *command_queue;

        uint32_t buffer_count = -1;
        std::vector<FrameContext> frames;
    };

    static D3D12Environment env;

    static void __stdcall hkExecuteCommandList(ID3D12CommandQueue *sc, UINT count, ID3D12CommandList **list) {
        if (!env.command_queue) {
            env.command_queue = sc;
        }

        oExecCmdList(sc, count, list);
    }

    static HRESULT __stdcall hkPresent(IDXGISwapChain3 *sc, UINT interval, UINT flags) {
        static bool init = false;
        static bool show_demo = true;

        static ID3D12DescriptorHeap *srv_heap = nullptr;

        DXGI_SWAP_CHAIN_DESC desc;
        ID3D12Device *device = nullptr;

        sc->GetDesc(&desc);
        sc->GetDevice(__uuidof(ID3D12Device), (void **)&device);

        if (!init) {
            if (env.command_queue) {

                D3D12_DESCRIPTOR_HEAP_DESC hd = {};

                {

                    hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                    hd.NumDescriptors = desc.BufferCount;
                    hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                    env.frames.resize(desc.BufferCount);

                    if (device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&env.desc_imgui_render)) != S_OK)
                        return tPresent(sc, interval, flags);
                }

                {
                    hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                    hd.NumDescriptors = desc.BufferCount;
                    hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                    hd.NodeMask = 1;

                    if (device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&env.desc_back_buffer)) != S_OK)
                        return tPresent(sc, interval, flags);
                }

                ID3D12CommandAllocator *cmd_allocator;
                if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd_allocator)) != S_OK)
                    return tPresent(sc, interval, flags);

                for (size_t i = 0; i < desc.BufferCount; i++) {
                    env.frames[i].command_allocator = cmd_allocator;
                }

                if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_allocator, NULL, IID_PPV_ARGS(&env.command_list)) != S_OK ||
                    env.command_list->Close() != S_OK)
                    return tPresent(sc, interval, flags);

                const auto rtv_desc_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = env.desc_back_buffer->GetCPUDescriptorHandleForHeapStart();

                for (size_t i = 0; i < desc.BufferCount; i++) {
                    ID3D12Resource *back_buffer = nullptr;
                    env.frames[i].desc_handle = rtv_handle;
                    sc->GetBuffer(i, IID_PPV_ARGS(&back_buffer));
                    device->CreateRenderTargetView(back_buffer, nullptr, rtv_handle);
                    env.frames[i].resource = back_buffer;
                    rtv_handle.ptr += rtv_desc_size;
                }

                ImGui::CreateContext();

                auto &io = ImGui::GetIO();
                io.IniFilename = nullptr;
                io.LogFilename = nullptr;

                ImGui_ImplWin32_EnableDpiAwareness();
                ImGui_ImplWin32_Init(desc.OutputWindow);

                ImGui_ImplDX12_Init(
                    device,
                    desc.BufferCount,
                    DXGI_FORMAT_R8G8B8A8_UNORM,
                    env.desc_imgui_render,
                    env.desc_imgui_render->GetCPUDescriptorHandleForHeapStart(),
                    env.desc_imgui_render->GetGPUDescriptorHandleForHeapStart());

                ImGui_ImplDX12_CreateDeviceObjects();

                menu::watch(desc.OutputWindow);

                init = true;
            }
        } else {

            if (env.command_queue) {

                ImGui_ImplDX12_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                ImGui::ShowDemoWindow(&show_demo);

                // menu::render();

                ImGui::EndFrame();

                auto &frame = env.frames[sc->GetCurrentBackBufferIndex()];
                frame.command_allocator->Reset();

                D3D12_RESOURCE_BARRIER barrier;
                {
                    std::memset(&barrier, 0, sizeof(barrier));
                    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                    barrier.Transition.pResource = frame.resource;
                    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                }

                env.command_list->Reset(frame.command_allocator, nullptr);
                env.command_list->ResourceBarrier(1, nullptr);
                env.command_list->OMSetRenderTargets(1, &frame.desc_handle, FALSE, nullptr);
                env.command_list->SetDescriptorHeaps(1, &env.desc_imgui_render);

                ImGui::Render();
                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), env.command_list);

                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                env.command_list->ResourceBarrier(1, &barrier);
                env.command_list->Close();
                env.command_queue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList *const *>(&env.command_list));
            }
        }

        return oPresent(sc, interval, flags);
    }

    bool install() {

        IDXGIFactory *factory = nullptr;
        IDXGIAdapter *adapter = nullptr;
        ID3D12Device *device = nullptr;
        IDXGISwapChain *swapchain = nullptr;

        ID3D12CommandQueue *cmd_queue = nullptr;
        ID3D12CommandAllocator *cmd_allocator = nullptr;
        ID3D12GraphicsCommandList *cmd_list = nullptr;

        D3D12_COMMAND_QUEUE_DESC cd;
        {
            std::memset(&cd, 0, sizeof(cd));
            cd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            cd.Priority = 0;
            cd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            cd.NodeMask = 0;
        }

        DXGI_SWAP_CHAIN_DESC sd;
        {
            std::memset(&sd, 0, sizeof(sd));
            sd.BufferDesc =
                {
                    100, 100, {60, 1}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED};
            sd.SampleDesc = {1, 0};
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 2;
            sd.OutputWindow = GetForegroundWindow();
            sd.Windowed = TRUE;
            sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        }

        if (CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory) != S_OK) {
            goto fail;
        }

        if (factory->EnumAdapters(0, &adapter) != S_OK) {
            goto fail;
        }

        if (D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)) != S_OK) {
            goto fail;
        }

        if (device->CreateCommandQueue(&cd, __uuidof(ID3D12CommandQueue), (void **)&cmd_queue) != S_OK) {
            goto fail;
        }

        if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&cmd_allocator) != S_OK) {
            goto fail;
        }

        if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_allocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&cmd_list) != S_OK) {
            goto fail;
        }

        if (factory->CreateSwapChain(cmd_queue, &sd, &swapchain) != S_OK) {
            goto fail;
        }

        void **vmt0 = *(void ***)swapchain;
        void **vmt1 = *(void ***)cmd_queue;

        RELEASE(device);
        RELEASE(cmd_queue);
        RELEASE(cmd_allocator);
        RELEASE(cmd_list);
        RELEASE(swapchain);
        return cl::hook::create(vmt0[8], hkPresent, (void **)(&oPresent)) &&
               cl::hook::create(vmt1[10], hkExecuteCommandList, (void **)(&oExecCmdList));

    fail:
        RELEASE(device);
        RELEASE(cmd_queue);
        RELEASE(cmd_allocator);
        RELEASE(cmd_list);
        RELEASE(swapchain);
        return false;
    }

    void uninstall() {
        menu::unwatch();
    }

}