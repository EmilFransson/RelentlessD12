#include "HorizontalBox.h"

namespace Relentless
{
	HorizontalBox::HorizontalBox(bool isChildRegion, const Vector2& size) noexcept
		:m_Size{size}
		,m_IsChildRegion{isChildRegion}
	{
	}

	Ref<IBaseWidget> HorizontalBox::GetChild(uint32 index) const noexcept
	{
		return m_Children[index];
	}

	float HorizontalBox::CalcDesiredWidth() const noexcept
	{
		float totalWidth = 0.0f;
		uint32 count = static_cast<int>(m_Children.size());

		for (const auto& child : m_Children)
		{
			if (child->GetSizePolicy() == ESizePolicy::Fixed || child->GetSizePolicy() == ESizePolicy::Auto)
				totalWidth += child->CalcDesiredWidth();

			// Stretch widgets defer size to layout; they report 0 here
		}

		if (count > 1)
			totalWidth += ImGui::GetStyle().ItemSpacing.x * (count - 1);

		return totalWidth;
	}

	bool HorizontalBox::HasWidget(Ref<IBaseWidget> pWidget) noexcept
	{
		return std::find(m_Children.begin(), m_Children.end(), pWidget) != m_Children.end();
	}

	void HorizontalBox::SetAlignmentPolicy(EAlignmentPolicy alignmentPolicy) noexcept
	{
		m_Alignment = alignmentPolicy;
	}

	void HorizontalBox::SetMargin(const FloatRect& margin) noexcept
	{
		m_Margin = margin;
	}

	void HorizontalBox::SetSize(const Vector2& size) noexcept
	{
		m_Size = size;
	}

	void HorizontalBox::OnRender() noexcept
	{
		ImVec2 currentPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ currentPos.x + m_Margin.Left, currentPos.y + m_Margin.Top });

		if (m_IsChildRegion)
			ImGui::BeginChild(std::format("##{}-child", (long)this).c_str(), ImVec2(m_Size.x,m_Size.y), false, GetFlags());

		float totalFixedWidth = 0.0f;
		int numStretch = 0;

		for (auto& child : m_Children)
		{
			if (child->GetSizePolicy() == ESizePolicy::Stretch)
				++numStretch;
			else
				totalFixedWidth += child->CalcDesiredWidth();
		}

		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float totalSpacing = spacing * (m_Children.size() - 1);
		const float availableWidth = ImGui::GetContentRegionAvail().x - m_Margin.Right;
		const float stretchWidth = (availableWidth - totalFixedWidth - totalSpacing) / Math::Max(numStretch, 1);

		// Compute total used width
		const float totalContentWidth = totalFixedWidth + totalSpacing + stretchWidth * numStretch;

		// Apply alignment offset
		float offsetX = 0.0f;
		switch (m_Alignment)
		{
		case EAlignmentPolicy::Left:
			offsetX = 0.0f;
			break;
		case EAlignmentPolicy::Center:
			offsetX = (availableWidth - totalContentWidth) * 0.5f;
			break;
		case EAlignmentPolicy::Right:
			offsetX = (availableWidth - totalContentWidth);
			break;
		}

		// Apply offset to cursor
		if (offsetX > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

		ImVec2 boundsMin(+FLT_MAX, +FLT_MAX), boundsMax(-FLT_MAX, -FLT_MAX);

		auto&& UpdateBounds = [&]()
			{
				ImVec2 cmin = ImGui::GetItemRectMin();
				ImVec2 cmax = ImGui::GetItemRectMax();
				boundsMin.x = Math::Min(boundsMin.x, cmin.x);
				boundsMin.y = Math::Min(boundsMin.y, cmin.y);
				boundsMax.x = Math::Max(boundsMax.x, cmax.x);
				boundsMax.y = Math::Max(boundsMax.y, cmax.y);
			};

		// Step 2: layout pass
		ImGui::BeginGroup();
		for (size_t i = 0; i < m_Children.size(); ++i)
		{
			Ref<IBaseWidget>& child = m_Children[i];

			const float width = (child->GetSizePolicy() == ESizePolicy::Stretch) ? stretchWidth : child->CalcDesiredWidth();

			ImGui::SetNextItemWidth(width);
			child->Render();

			UpdateBounds();

			if (i < m_Children.size() - 1)
				ImGui::SameLine(0.0f, spacing);
		}
		ImGui::EndGroup();

		if (GetSizePolicy() == ESizePolicy::Stretch)
		{
			const float fillWidth = ImGui::GetContentRegionAvail().x;
			ImGui::Dummy(ImVec2(fillWidth, 0.0f));
			// grab the last‐rendered widget’s rect in screen coords
			UpdateBounds();
		}

		auto minRect = ImGui::GetItemRectMin();
		auto maxRect = ImGui::GetItemRectMax();

		const bool rectHovered = ImGui::IsMouseHoveringRect(boundsMin, boundsMax);
		//ImGui::GetWindowDrawList()->AddRect(minRect, maxRect, ImColor(1.0f, 1.0f, 1.0f, 1.0f));

		if (!this->m_IsHovered && rectHovered)
			this->OnMouseEnter_private();
		else if (this->m_IsHovered && !rectHovered)
			this->OnMouseExit_private();

		if (m_IsChildRegion)
			ImGui::EndChild();

		currentPos = ImGui::GetCursorPos();
		ImGui::SetCursorPosY(currentPos.y + m_Margin.Bottom);
	}

	void HorizontalBox::SetWidthConstraint(float width) noexcept
	{
		IWidget<HorizontalBox>::SetWidthConstraint(width);

		for (auto& pChild : m_Children)
			pChild->SetWidthConstraint(width);
	}

}
