#include "Separator.h"

namespace Relentless
{
	Separator::Separator() noexcept
	{
		SetHorizontalSizePolicy(ESizePolicy::Stretch);
		SetVerticalSizePolicy(ESizePolicy::Stretch);
		SetPadding(Vector2::Zero);
	}

	float Separator::GetThickness() const noexcept
	{
		return m_Thickness;
	}

	void Separator::OnRender() noexcept
	{
		ImGui::SeparatorEx(m_IsHorizontal ? ImGuiSeparatorFlags_Horizontal : ImGuiSeparatorFlags_Vertical, m_Thickness);
	}

	Vector2 Separator::ReportSize() const noexcept
	{
		return m_IsHorizontal ? Vector2(0.0f, Math::Max(m_Thickness, 1.0f)) : Vector2(Math::Max(m_Thickness, 1.0f), 0.0f);
	}
	
	Separator* Separator::SetActiveColor(const Color& aColor) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Separator, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return this;
	}

	Separator* Separator::SetThickness(float aThickness) noexcept
	{
		m_Thickness = aThickness;
		return this;
	}
}