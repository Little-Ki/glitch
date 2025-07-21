#include "menu_hook.h"

#define RELEASE(p) if (p) { p->Release(); p = nullptr; } 

#include <Windows.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <iostream>

#include "lib_hook.h"
#include "menu.h"
#include "menu_win32.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "menu_imgui_win32.h"

namespace ct::menu::hook {

	using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
	using ExecuteCommandListFn = void(__stdcall*)(ID3D12CommandQueue*, UINT, ID3D12CommandList**);

	static PresentFn oPresent = nullptr;
	static PresentFn tPresent = nullptr;

	static ExecuteCommandListFn oExecCmdList = nullptr;
	static ExecuteCommandListFn tExecCmdList = nullptr;

	static ID3D12CommandQueue* gCommandQueue = nullptr;

	class D3D12HeapAllocator
	{
		ID3D12DescriptorHeap*		heap = nullptr;
		D3D12_DESCRIPTOR_HEAP_TYPE  type = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		D3D12_CPU_DESCRIPTOR_HANDLE start_cpu;
		D3D12_GPU_DESCRIPTOR_HANDLE start_gpu;
		UINT                        handle_increment;
		ImVector<int>               free_indices;
	public:

		void create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
		{
			IM_ASSERT(heap == nullptr && free_indices.empty());
			this->heap = heap;
			D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
			type = desc.Type;
			start_cpu = heap->GetCPUDescriptorHandleForHeapStart();
			start_gpu = heap->GetGPUDescriptorHandleForHeapStart();
			handle_increment = device->GetDescriptorHandleIncrementSize(type);
			free_indices.reserve((int)desc.NumDescriptors);
			for (int n = desc.NumDescriptors; n > 0; n--)
				free_indices.push_back(n - 1);
		}

		void destroy()
		{
			heap = nullptr;
			free_indices.clear();
		}

		void alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
		{
			IM_ASSERT(free_indices.Size > 0);
			int idx = free_indices.back();
			free_indices.pop_back();
			out_cpu_desc_handle->ptr = start_cpu.ptr + (idx * handle_increment);
			out_gpu_desc_handle->ptr = start_gpu.ptr + (idx * handle_increment);
		}

		void free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
		{
			int cpu_idx = (int)((out_cpu_desc_handle.ptr - start_cpu.ptr) / handle_increment);
			int gpu_idx = (int)((out_gpu_desc_handle.ptr - start_gpu.ptr) / handle_increment);
			IM_ASSERT(cpu_idx == gpu_idx);
			free_indices.push_back(cpu_idx);
		}
	};

	static void __stdcall hkExecuteCommandList(ID3D12CommandQueue* self, UINT count, ID3D12CommandList** list) {
		if (gCommandQueue != self) {
			gCommandQueue = self;
		}

		tExecCmdList(self, count, list);
	}

	static HRESULT __stdcall hkPresent(IDXGISwapChain* self, UINT sync_interval, UINT flags)
	{

		static bool init = false;
		static bool show_demo = true;

		static D3D12HeapAllocator allocator;

		static ID3D12DescriptorHeap* srv_heap = nullptr;

		DXGI_SWAP_CHAIN_DESC desc;

		ID3D12Device* device{ nullptr };

		self->GetDesc(&desc);
		self->GetDevice(__uuidof(ID3D12Device), (void**)&device);

		if (!init)
		{
			if (cmd_queue) {

				ImGui_ImplDX12_InitInfo init_info{};
				{
					std::memset(&init_info, 0, sizeof(init_info));
					init_info.Device = device;
					init_info.CommandQueue = cmd_queue;
					init_info.NumFramesInFlight = 2;
					init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
					init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
					init_info.SrvDescriptorHeap = nullptr;
					init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return allocator.alloc(out_cpu_handle, out_gpu_handle); };
					init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return allocator.free(cpu_handle, gpu_handle); };
					
				}

				if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srv_heap)) != S_OK)
					return false;

				ImGui_ImplWin32_EnableDpiAwareness();
				ImGui_ImplDX12_Init(&init_info);

				menu::win32::install(desc.OutputWindow);

				ImGui::CreateContext();
				ImGui_ImplWin32_Init(desc.OutputWindow);
				ImGui_ImplDX11_Init(device, context);

				init = true;
			}
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

		return tPresent(self, sync_interval, flags);
	}

	bool install() {

		IDXGIFactory* factory = nullptr;
		IDXGIAdapter* adapter = nullptr;
		ID3D12Device* device = nullptr;
		IDXGISwapChain* swapchain = nullptr;

		ID3D12CommandQueue* cmd_queue = nullptr;
		ID3D12CommandAllocator* cmd_allocator = nullptr;
		ID3D12GraphicsCommandList* cmd_list = nullptr;

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
				100, 100,
				{60, 1},
				DXGI_FORMAT_R8G8B8A8_UNORM,
				DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
				DXGI_MODE_SCALING_UNSPECIFIED
			};
			sd.SampleDesc = { 1,0 };
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 2;
			sd.OutputWindow = GetForegroundWindow();
			sd.Windowed = TRUE;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}

		if (CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory) != S_OK) {
			goto fail;
		}

		if (factory->EnumAdapters(0, &adapter) != S_OK)
		{
			goto fail;
		}

		if (D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)) != S_OK) {
			goto fail;
		}

		if (device->CreateCommandQueue(&cd, __uuidof(ID3D12CommandQueue), (void**)&cmd_queue) != S_OK)
		{
			goto fail;
		}

		if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&cmd_allocator) != S_OK)
		{
			goto fail;
		}

		if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_allocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&cmd_list) != S_OK)
		{
			goto fail;
		}

		if (factory->CreateSwapChain(cmd_queue, &sd, &swapchain) != S_OK)
		{
			goto fail;
		}

		void** vmt = *(void***)swapchain;

		oPresent = reinterpret_cast<PresentFn>(vmt[8]);

		RELEASE(device);
		RELEASE(cmd_queue);
		RELEASE(cmd_allocator);
		RELEASE(cmd_list);
		RELEASE(swapchain);
		return cl::hook::create(oPresent, hkPresent, (void**)(&tPresent));

	fail:
		RELEASE(device);
		RELEASE(cmd_queue);
		RELEASE(cmd_allocator);
		RELEASE(cmd_list);
		RELEASE(swapchain);
		return false;
	}

	void uninstall() {
		cl::hook::release(oPresent);
		menu::win32::uninstall();
	}

}