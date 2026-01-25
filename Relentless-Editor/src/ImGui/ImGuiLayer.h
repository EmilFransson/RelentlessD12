#pragma once
#include <Relentless.h>
#include "ImGui/ImGuiIncludes.h"

namespace Relentless
{
	#define OPENSANS_BOLD_18 0

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(GraphicsDevice* pDevice) noexcept;
		virtual ~ImGuiLayer() noexcept override = default;

		void BeginFrame(Ref<Texture> pTarget, CommandContext* pCommandContext) noexcept;
		void EndFrame(CommandContext* pCommandContext) noexcept;

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
