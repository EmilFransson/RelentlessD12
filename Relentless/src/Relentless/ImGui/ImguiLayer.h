#pragma once
#include "../Events/Layer.h"
#include "../Graphics/DescriptorHeap.h"
//#include "imgui.h"
//#include "backends/imgui_impl_win32.h"
//#include "backends/imgui_impl_dx12.h"
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
	private:
		virtual void OnAttach() override final;
		virtual void OnEvent(IEvent&) noexcept override final{};
	private:
		std::unique_ptr<DescriptorHeap> m_pDescriptorHeap;
	};
}