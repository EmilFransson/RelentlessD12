#include "VerticalBox.h"

namespace Relentless
{
	VerticalBox::VerticalBox(const Vector2& size, bool isChildRegion) noexcept
		:m_Size{size}
		,m_IsChildRegion{isChildRegion}
	{
		m_Spacing = Vector2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y);
	}

	float VerticalBox::CalcDesiredWidth() const noexcept
	{
		float maxWidth = 0.0f;

		for (const auto& pChild : m_Children)
		{
			if (pChild->GetSizePolicy() != ESizePolicy::Stretch)
				maxWidth = Math::Max(maxWidth, pChild->CalcDesiredWidth());
		}

		return maxWidth;
	}

	bool VerticalBox::HasWidget(Ref<IBaseWidget> pWidget) noexcept
	{
		return std::find(m_Children.begin(), m_Children.end(), pWidget) != m_Children.end();
	}

	void VerticalBox::SetIsChildRegion(bool state) noexcept
	{
		m_IsChildRegion = state;
	}

	void VerticalBox::SetSpacing(const Vector2& aSpacing) noexcept
	{
		m_Spacing = aSpacing;
	}

	void VerticalBox::OnRender() noexcept
	{
		if (m_IsChildRegion)
			ImGui::BeginChild(std::format("##{}-child", (long)this).c_str(), ImVec2(m_Size.x, m_Size.y), false, GetFlags());

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(m_Spacing.x, m_Spacing.y));

		for (auto& pChild : m_Children)
			pChild->Render();

		ImGui::PopStyleVar();

		if (m_IsChildRegion)
		{
			const bool isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

			if (m_IsFocused != isFocused)
			{
				m_IsFocused = isFocused;
				OnFocusChanged(m_IsFocused);
			}

			ImGui::EndChild();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		}
	}
}
