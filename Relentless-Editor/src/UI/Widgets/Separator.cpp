#include "Separator.h"

namespace Relentless
{
	Separator::Separator(bool aIsHorizontal) noexcept
		: m_IsHorizontal{ aIsHorizontal }
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
		if (m_IsHorizontal)
			ImGui::SeparatorEx(m_IsHorizontal ? ImGuiSeparatorFlags_Horizontal : ImGuiSeparatorFlags_Vertical, m_Thickness);
		else
		{
			const ImVec2 position = ImGui::GetCursorScreenPos();
			const float height = GetAssignedSize().y;
			
			ImGui::GetWindowDrawList()->AddLine(ImVec2(position.x, position.y), ImVec2(position.x, position.y + height), ImGui::GetColorU32(ImGuiCol_Separator), m_Thickness);
			ImGui::Dummy(ImVec2(m_Thickness, height));

			const float visualHalf = m_Thickness * 0.5f;
			const float hitHalf = Math::Max(visualHalf, 4.0f);
			m_Rect = ImRect(ImVec2(position.x - hitHalf, position.y), ImVec2(position.x + hitHalf, position.y + height));
		}
	}

	Vector2 Separator::ReportSize() const noexcept
	{
		return m_IsHorizontal ? Vector2(3.0f, Math::Max(m_Thickness, 3.0f)) : Vector2(Math::Max(m_Thickness, 3.0f), 3.0f);
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

	bool Separator::RequiresAssignedSize() const noexcept
	{
		return !m_IsHorizontal;
	}

	bool Separator::TryGetHoverRect(ImRect& aRect) const noexcept
	{
		if (m_IsHorizontal)
			return false;

		aRect = m_Rect;

		return true;
	}
}