#include "HorizontalBox.h"

namespace Relentless
{
	HorizontalBox::HorizontalBox(std::string_view id) noexcept
		: IWidget{ id }
	{
	}

	void HorizontalBox::Add(Ref<IWidget> pWidget) noexcept
	{
		RLS_ASSERT(!HasWidget(pWidget), "[HorizontalBox::Add] Widget already assigned as child.");
		m_Children.push_back(pWidget);
	}

	bool HorizontalBox::HasWidget(Ref<IWidget> pWidget) noexcept
	{
		return std::find(m_Children.begin(), m_Children.end(), pWidget) != m_Children.end();
	}

	void HorizontalBox::OnRender() noexcept
	{
		float maxItemWidth = 180.0f;
		float rightPadding = 4.0f;
		float spacing = ImGui::GetStyle().ItemSpacing.x;
		size_t count = m_Children.size();
		if (count == 0)
			return;

		float totalAvail = ImGui::GetContentRegionAvail().x;
		float totalSpacing = spacing * (count - 1);
		float totalWidgetSpace = totalAvail - totalSpacing - rightPadding;
		float itemWidth = ImMin(totalWidgetSpace / count, maxItemWidth);

		for (size_t i = 0u; i < m_Children.size(); ++i)
		{
			ImGui::SetNextItemWidth(itemWidth);
			m_Children[i]->Render();
			
			if (i != m_Children.size() - 1)
				ImGui::SameLine();
		}
	}

	void HorizontalBox::SetWidthConstraint(float width) noexcept
	{
		IWidget::SetWidthConstraint(width);

		for (auto& pChild : m_Children)
			pChild->SetWidthConstraint(width);
	}

}
