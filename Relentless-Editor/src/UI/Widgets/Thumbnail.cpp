#include "Thumbnail.h"

namespace Relentless
{
	Vector2 Thumbnail::ReportSize() const noexcept
	{
		return m_Size;
	}

	void Thumbnail::SetBrush(const ThumbnailBrush& aBrush) noexcept
	{
		m_Brush = aBrush;
	}

	void Thumbnail::SetSize(const Vector2& aSize) noexcept
	{
		m_Size = aSize;
	}

	void Thumbnail::OnRender() noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::ImageButton("##Thumbnail", (ImTextureID)m_Brush.BackingTexture->GetSRV()->GetGPUHandle().ptr, 
			ImVec2(m_Size.x, m_Size.y), ImVec2(0, 0), ImVec2(1,1), 
			ImVec4(m_Brush.BackgroundColor.R(), m_Brush.BackgroundColor.G(), m_Brush.BackgroundColor.B(), m_Brush.BackgroundColor.A()),
			ImVec4(m_Brush.TintColor.R(), m_Brush.TintColor.G(), m_Brush.TintColor.B(), m_Brush.TintColor.A()));
		ImGui::PopStyleVar();

		const ImVec2 minPoint = ImGui::GetItemRectMin();
		const ImVec2 maxPoint = ImGui::GetItemRectMax();

		const ImU32 lineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(m_Brush.LineColor.R(), m_Brush.LineColor.G(), m_Brush.LineColor.B(), m_Brush.LineColor.A()));
		pDrawList->AddLine(ImVec2(minPoint.x, maxPoint.y - 2.0f), ImVec2(maxPoint.x, maxPoint.y - 2.0f), lineColor, 2.0f);
	}
}