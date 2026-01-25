 #pragma once
// #include "EventSystem/Layer.h"
// #include "Graphics/RHI/RHI.h"
// #include "Graphics/RHI/DescriptorHeap.h"
// 
// #include <ImGUI/imgui.h>
// #include <ImGUI/imgui_internal.h>
// #include <ImGUI/backends/imgui_impl_win32.h>
// #include <ImGUI/backends/imgui_impl_dx12.h>
// #include <ImGuizmo/ImGuizmo.h>
// 
// namespace Relentless
// {
// 	#define OPENSANS_BOLD_18 0
// 
// 	class ImguiLayer : public Layer 
// 	{
// 	public:
// 		ImguiLayer(GraphicsDevice* pDevice) noexcept;
// 		virtual ~ImguiLayer() noexcept override = default;
// 
// 		void BeginFrame(Ref<Texture> pTarget, CommandContext* pCommandContext) noexcept;
// 		void EndFrame(CommandContext* pCommandContext) noexcept;
// 
// 		virtual void OnImGuiRender() noexcept override final;
// 	private:
// 		virtual void OnAttach() override final;
// 		virtual void OnDetach() override final;
// 	private:
// 		GraphicsDevice* m_pDevice = nullptr;
// 		UniquePtr<DescriptorHeap> m_pDescriptorHeap = nullptr;
// 		DescriptorHandle m_DescriptorHandle;
// 	};
// }
