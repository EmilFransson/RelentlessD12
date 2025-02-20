#include "ViewportPanel.h"
#include "../Core/Editor.h"

namespace Relentless
{
	ViewportPanel::ViewportPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor, uint32 renderViewIndex) noexcept
		: PanelBase(pName, flags), m_pEditor{pEditor}, m_RenderViewIndex{renderViewIndex}
	{}

	void ViewportPanel::PreRender() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	}

	void ViewportPanel::OnRender() noexcept
	{
		const Vector2u& region = GetContentRegionAvail();
		if (region.x == 0u || region.y == 0u)
			return;

		ViewportRenderView& renderView = m_pEditor->GetRenderView(m_RenderViewIndex);
		ImGui::Image((ImTextureID)renderView.pTarget->GetSRV()->GetGPUHandle().ptr, ImVec2(region.x, region.y));
	}

	void ViewportPanel::PostRender() noexcept
	{
		ImGui::PopStyleVar();
	}
}