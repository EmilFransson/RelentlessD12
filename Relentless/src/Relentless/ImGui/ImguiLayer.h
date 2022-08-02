#pragma once
#include "../Events/Layer.h"
#include "../Graphics/DescriptorHeap.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"
namespace Relentless
{
	class ImguiLayer : public Layer 
	{
	public:
		ImguiLayer() noexcept;
		virtual ~ImguiLayer() noexcept override final;
		static void BeginFrame() noexcept;
		static void EndFrame() noexcept;
		virtual void OnImGuiRender() noexcept override final;
		static constexpr Microsoft::WRL::ComPtr<ID3D12Resource>& GetUITexture() noexcept { return m_pUITexture; }
		static constexpr D3D12_CPU_DESCRIPTOR_HANDLE& GetUITextureCPUHandle() noexcept { return my_texture_srv_cpu_handle; }
		static constexpr D3D12_GPU_DESCRIPTOR_HANDLE& GetUITextureGPUHandle() noexcept { return my_texture_srv_gpu_handle; }
		static void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
	private:
		virtual void OnAttach() override final;
		virtual void OnEvent(IEvent&) noexcept override final{};
	private:
		static std::unique_ptr<DescriptorHeap> m_pDescriptorHeap;
		static Microsoft::WRL::ComPtr<ID3D12Resource> m_pUITexture;
		static D3D12_CPU_DESCRIPTOR_HANDLE my_texture_srv_cpu_handle;
		static D3D12_GPU_DESCRIPTOR_HANDLE my_texture_srv_gpu_handle;
	};
}