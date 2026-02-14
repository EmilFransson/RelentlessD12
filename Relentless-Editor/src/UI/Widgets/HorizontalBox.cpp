#include "HorizontalBox.h"

namespace Relentless
{
	HorizontalBox::HorizontalBox(const Vector2 aSize, bool aIsChildRegion) noexcept
		: IWidgetContainer<HorizontalBox>(aSize, aIsChildRegion)
	{
		SetSpacing(8.0f);
	}

	Vector2 HorizontalBox::ReportSize() const noexcept
	{
		if (GetHorizontalSizePolicy() == ESizePolicy::Fixed)
			return m_Size;

		float totalWidth = 0.0f;
		float maxHeight = 0.0f;
		uint32 visibleCount = 0u;

		for (const auto& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible())
				continue;

			const Vector2 size = pWidget->ReportSize();
			const FloatRect margin = pWidget->GetMargin();

			totalWidth += (size.x + margin.Left + margin.Right);
			maxHeight = Math::Max(maxHeight, size.y + margin.Top + margin.Bottom);

			visibleCount++;
		}

		if (visibleCount > 1)
			totalWidth += m_Spacing.x * (visibleCount - 1u);

		const FloatRect& padding = GetPadding();
		totalWidth += padding.Left + padding.Right;
		maxHeight += padding.Top + padding.Bottom;

		return Vector2(totalWidth, maxHeight);
	}

	HorizontalBox* HorizontalBox::SetSpacing(float aSpacing) noexcept
	{
		m_Spacing = Vector2(aSpacing, 0.0f);
		return this;
	}

	void HorizontalBox::OnRender() noexcept
	{
		if (!IsVisible())
			return;

		const FloatRect& padding = GetPadding();
		const ImVec2 basePosition = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ basePosition.x + padding.Left, basePosition.y + padding.Top });

		ImGuiWindowFlags boxFlags = 0;
		if (!IsScrollBarsVisible())
			boxFlags |= ImGuiWindowFlags_NoScrollbar;
		if (!IsMouseScrollingEnabled())
			boxFlags |= ImGuiWindowFlags_NoScrollWithMouse;

		if (m_IsChildRegion)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			const Vector2 targetSize = HasAssignedSize() ? GetAssignedSize() : m_Size;
			const Vector2 innerSize = Vector2(Math::Max(0.0f, targetSize.x - (padding.Left + padding.Right)), Math::Max(0.0f, targetSize.y - (padding.Top + padding.Bottom)));

			ImGui::BeginChild(ImGui::GetID(this), ImVec2(innerSize.x, innerSize.y), false, boxFlags);
		}

		// ---- MEASURE pass (children desired, with margins) ----
		struct ChildMeasure 
		{ 
			IBaseWidget* pWidget; 
			Vector2 Size; 
			FloatRect Margin; 
			bool Stretch; 
		};

		std::vector<ChildMeasure> list; 
		list.reserve(m_Widgets.size());

		float totalFixedWidth = 0.0f;
		float rowMaxHeight = 0.0f;
		int   numStretch = 0;

		for (const auto& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible()) 
				continue;

			ChildMeasure measuredChild
			{ 
				pWidget,
				pWidget->ReportSize(),
				pWidget->GetMargin(),
				(pWidget->GetHorizontalSizePolicy() == ESizePolicy::Stretch)
			};

			if (measuredChild.Stretch)
				++numStretch;
			else            
				totalFixedWidth += measuredChild.Size.x + measuredChild.Margin.Left + measuredChild.Margin.Right;

			rowMaxHeight = Math::Max(rowMaxHeight, measuredChild.Size.y + measuredChild.Margin.Top + measuredChild.Margin.Bottom);
			list.push_back(measuredChild);
		}

		const int   numVisibleWidgets = (int)list.size();
		const float spacing = m_Spacing.x;
		const float totalSpacing = (numVisibleWidgets > 0) ? spacing * (numVisibleWidgets - 1) : 0.0f;

		// Available inner width from ImGui *at our current cursor* (cursor-only approach)
		const float innerAvailableWidth = Math::Max(0.0f, ImGui::GetContentRegionAvail().x - (m_IsChildRegion ? 0.0f : padding.Right));

		// Stretch slice on X
		const float stretchSlice = (innerAvailableWidth - totalFixedWidth - totalSpacing) / Math::Max(numStretch, 1);

		const float totalContentWidth = totalFixedWidth + totalSpacing + Math::Max(0.0f, stretchSlice) * numStretch;

		const EHorizontalAlignmentPolicy alignment = GetHorizontalAlignmentPolicy();
		// Alignment offset on X
		float offsetX = 0.0f;
		if (alignment == EHorizontalAlignmentPolicy::Center)
			offsetX = Math::Max(0.0f, (innerAvailableWidth - totalContentWidth) * 0.5f);
		else if (alignment == EHorizontalAlignmentPolicy::Right)
			offsetX = Math::Max(0.0f, (innerAvailableWidth - totalContentWidth));

		// ---- LAYOUT pass (cursor only) ----
		// Use ONE spacing mechanism: ItemSpacing + SameLine()
		ScopedStyleVar itemStyle;
		itemStyle.Push(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, 0.0f));

		// Move to aligned start
		if (offsetX > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

		ImVec2 boundsMin(+FLT_MAX, +FLT_MAX), boundsMax(-FLT_MAX, -FLT_MAX);
		auto UpdateBounds = [&]() 
			{
				ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
				boundsMin.x = std::min(boundsMin.x, a.x); boundsMin.y = std::min(boundsMin.y, a.y);
				boundsMax.x = std::max(boundsMax.x, b.x); boundsMax.y = std::max(boundsMax.y, b.y);
			};

		ImGui::BeginGroup();

		bool first = true;
		for (const auto& pVisibleChild : list)
		{
			// Inter-child spacing from ItemSpacing via SameLine()
			if (!first)
				ImGui::SameLine();
			
			first = false;

			// Apply LEFT/TOP margins via cursor offsets
			if (pVisibleChild.Margin.Left > 0.0f)
			{
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(pVisibleChild.Margin.Left, 0.0f));
			}
			if (pVisibleChild.Margin.Top > 0.0f)
				ImGui::Dummy(ImVec2(0.0f, pVisibleChild.Margin.Top));

			//ImVec2 currentCursorPos = ImGui::GetCursorPos();
			//ImGui::SetCursorPos({ currentCursorPos.x + pVisibleChild.Margin.Left, currentCursorPos.y + pVisibleChild.Margin.Top });

			// Compute child content width (stretch or fixed)
			const float contentWidth = pVisibleChild.Stretch ? Math::Max(0.0f, stretchSlice - (pVisibleChild.Margin.Left + pVisibleChild.Margin.Right)) : pVisibleChild.Size.x;

			// Vertical centering inside row if needed (row height is rowMaxH)
			if (GetVerticalAlignmentPolicy() == EVerticalAlignmentPolicy::Center)
			{
				const float totalHeight = pVisibleChild.Size.y + pVisibleChild.Margin.Top + pVisibleChild.Margin.Bottom;
				const float addY = Math::Max(0.0f, (rowMaxHeight - totalHeight) * 0.5f);
				if (addY > 0.0f)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + addY);
			}

			// Give typical ImGui widgets the width hint
			ImGui::SetNextItemWidth(contentWidth);

			// If the child is a CONTAINER (needs a bounded “available region”), wrap it
			// so it "sees" precisely contentW x cm.d.y (or rowMaxH minus margins) as its space:
			if (pVisibleChild.pWidget->IsContainer())
			{
				const float childHeight = Math::Max(0.0f, rowMaxHeight - (pVisibleChild.Margin.Top + pVisibleChild.Margin.Bottom)); // or cm.d.y if fixed
				pVisibleChild.pWidget->AssignSize(Vector2(contentWidth, childHeight));
			}
			if (pVisibleChild.pWidget->RequiresAssignedSize())
			{
				const float childHeight = rowMaxHeight > 0.0f ? Math::Max(0.0f, rowMaxHeight - (pVisibleChild.Margin.Top + pVisibleChild.Margin.Bottom)) : ImGui::GetContentRegionAvail().y;
				pVisibleChild.pWidget->AssignSize(Vector2(contentWidth, childHeight));
			}
			
			pVisibleChild.pWidget->Render();

			UpdateBounds();

			// Advance by RIGHT/BOTTOM margins without adding extra spacing:
			// we can nudge X by adding a zero-height dummy of width=Right margin.
			if (pVisibleChild.Margin.Right > 0.0f)
			{
				ImGui::SameLine(0.0f, 0.0f);
				ImGui::Dummy(ImVec2(pVisibleChild.Margin.Right, 0.0f));
			}

			if (pVisibleChild.Margin.Bottom > 0.0f)
				ImGui::Dummy(ImVec2(0.0f, pVisibleChild.Margin.Bottom));
		}

		ImGui::EndGroup();

		// If HBox itself is stretch on X, consume remaining width (so parent flow continues correctly)
		if (GetHorizontalSizePolicy() == ESizePolicy::Stretch && !HasAssignedSize())
		{
			const float toConsume = ImGui::GetContentRegionAvail().x;
			if (toConsume > 0.0f)
				ImGui::Dummy(ImVec2(toConsume, 0.0f));
		}

		// Hover union
		if (numVisibleWidgets > 0)
		{
			const bool hovered = ImGui::IsMouseHoveringRect(boundsMin, boundsMax);
			if (!m_IsHovered && hovered)
				OnMouseEnter_private();
			else if (m_IsHovered && !hovered)
				OnMouseExit_private();
		}

		// Close outer child region (if any)
		if (m_IsChildRegion)
		{
			const bool isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

			if (m_IsFocused != isFocused)
			{
				m_IsFocused = isFocused;
				OnFocusChanged(m_IsFocused);
			}

			ImGui::PopStyleVar();
			ImGui::EndChild();
		}

		// Advance parent cursor by bottom padding
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.Bottom);
	}
}