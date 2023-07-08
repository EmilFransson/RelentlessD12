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
	private:
		virtual void OnAttach() override final;
		virtual void OnEvent(IEvent&) noexcept override final{};
	};
}
