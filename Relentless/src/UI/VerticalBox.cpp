#include "VerticalBox.h"

namespace Relentless
{
	VerticalBox::VerticalBox(std::string_view id, const Vector2& size, bool isChildRegion) noexcept
		:IWidget{id}
		,m_Size{size}
		,m_IsChildRegion{isChildRegion}
	{
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

	bool VerticalBox::HasWidget(Ref<IWidget> pWidget) noexcept
	{
		return std::find(m_Children.begin(), m_Children.end(), pWidget) != m_Children.end();
	}

	void VerticalBox::SetIsChildRegion(bool state) noexcept
	{
		m_IsChildRegion = state;
	}

	void VerticalBox::OnRender() noexcept
	{
		if (m_IsChildRegion)
		{
			ImGui::BeginChild(std::format("{}-child", m_ID).c_str(), ImVec2(m_Size.x, m_Size.y));
		}

		for (auto& pChild : m_Children)
			pChild->Render();

		if (m_IsChildRegion)
		{
			ImGui::EndChild();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		}
	}
}
