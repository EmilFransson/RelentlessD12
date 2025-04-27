#pragma once
#include "EventSystem/Layer.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/DescriptorHeap.h"
#pragma warning(push, 0)
#include "../../vendor/includes/ImGUI/imgui.h"
#include "../../vendor/includes/ImGUI/imgui_internal.h"
#include "../../vendor/includes/ImGUI/backends/imgui_impl_win32.h"
#include "../../vendor/includes/ImGUI/backends/imgui_impl_dx12.h"
#include "../../vendor/includes/ImGuizmo/ImGuizmo.h"
#pragma warning(pop)
namespace Relentless
{
	#define OPENSANS_BOLD_18 0

	class ImguiLayer : public Layer 
	{
	public:
		ImguiLayer(GraphicsDevice* pDevice) noexcept;
		virtual ~ImguiLayer() noexcept override final = default;

		void BeginFrame(Ref<Texture> pTarget, CommandContext* pCommandContext) noexcept;
		void EndFrame(Ref<Texture> pTarget, CommandContext* pCommandContext) noexcept;

		virtual void OnImGuiRender() noexcept override final;
	private:
		virtual void OnAttach() override final;
		virtual void OnDetach() override final;
	private:
		GraphicsDevice* m_pDevice = nullptr;
		UniquePtr<DescriptorHeap> m_pDescriptorHeap = nullptr;
		DescriptorHandle m_DescriptorHandle;
	};
}
