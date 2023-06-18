#pragma once
#include "../EventSystem/Layer.h"
#include "../Graphics/DescriptorHeap.h"
#pragma warning(push, 0)
#include "../../vendor/includes/ImGUI/imgui.h"
#include "../../vendor/includes/ImGUI/imgui_internal.h"
#include "../../vendor/includes/ImGUI/backends/imgui_impl_win32.h"
#include "../../vendor/includes/ImGUI/backends/imgui_impl_dx12.h"
#include "../../vendor/includes/ImGuizmo/ImGuizmo.h"
#pragma warning(pop)
#include "../Graphics/Resources/Texture.h"
namespace Relentless
{
#define OPENSANS_BOLD_18 0

	class ImguiLayer : public Layer 
	{
	public:
		ImguiLayer() noexcept;
		virtual ~ImguiLayer() noexcept override final;
		static void BeginFrame() noexcept;
		static void EndFrame() noexcept;
		virtual void OnImGuiRender() noexcept override final;
		static constexpr const std::shared_ptr<RenderTexture>& GetUITexture() noexcept { return m_pUITexture; }
		static constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetUITextureCPUHandle() noexcept { return m_pUITexture->GetSRVDescriptorHandle().CPUHandle; }
		static constexpr D3D12_GPU_DESCRIPTOR_HANDLE GetUITextureGPUHandle() noexcept { return m_pUITexture->GetSRVDescriptorHandle().GPUHandle; }
		static void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
	private:
		virtual void OnAttach() override final;
		virtual void OnEvent(IEvent&) noexcept override final{};
	private:
		static std::shared_ptr<RenderTexture> m_pUITexture;
	};
}
