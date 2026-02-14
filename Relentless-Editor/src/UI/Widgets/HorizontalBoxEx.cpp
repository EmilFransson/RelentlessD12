#include "HorizontalBoxEx.h"

namespace Relentless
{
	Vector2 HorizontalBoxEx::ReportSize() const noexcept
	{
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();
		const bool isFixedWidth = horizontalSizePolicy == ESizePolicy::Fixed;
		const bool isFixedHeight = verticalSizePolicy == ESizePolicy::Fixed;

		Vector2 size = Vector2::Zero;
		if (isFixedWidth)
			size.x = GetFixedWidth();
		if (isFixedHeight)
			size.y = GetFixedHeight();

		if (isFixedWidth && isFixedHeight)
			return size;

		float totalWidth = 0.0f;
		float maxHeight = 0.0f;
		uint32 visibleCount = 0u;

		for (const auto& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible())
				continue;

			const Vector2 widgetSize = pWidget->ReportSize();
			const FloatRect& margin = pWidget->GetMargin();

			totalWidth += (widgetSize.x + margin.Left + margin.Right);
			maxHeight = Math::Max(maxHeight, widgetSize.y + margin.Top + margin.Bottom);

			visibleCount++;
		}

		if (visibleCount > 1)
			totalWidth += m_Spacing.x * (visibleCount - 1u);

		const FloatRect& padding = GetPadding();
		totalWidth += padding.Left + padding.Right;
		maxHeight += padding.Top + padding.Bottom;

		if (!isFixedWidth)
			size.x = totalWidth;
		if (!isFixedHeight)
			size.y = maxHeight;

		return size;
	}

	HorizontalBoxEx* HorizontalBoxEx::SetSpacing(float aSpacing) noexcept
	{
		m_Spacing = Vector2(aSpacing, 0.0f);
		return this;
	}

	void HorizontalBoxEx::OnRender() noexcept
	{
		if (!IsVisible())
			return;

		const FloatRect& padding = GetPadding();
		
		//Final box size == Assigned size - padding.
		Vector2 boxSize = GetAssignedSize();
		boxSize.x -= (padding.Left + padding.Right);
		boxSize.y -= (padding.Top + padding.Bottom);
		boxSize.x = Math::Max(boxSize.x, 0.0f);
		boxSize.y = Math::Max(boxSize.y, 0.0f);
		
		const ImVec2 baseCursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(baseCursorPos.x + padding.Left, baseCursorPos.y + padding.Top));

		ImGuiWindowFlags windowFlags = 0;
		if (!IsMouseScrollingEnabled())
			windowFlags |= ImGuiWindowFlags_NoScrollWithMouse;
		if (IsHorizontalScrollBarEnabled())
			windowFlags |= ImGuiWindowFlags_HorizontalScrollbar;
		if (!IsScrollBarsVisible())
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

		const ImGuiChildFlags childFlags = 0;

		ScopedStyleVar styleVarScope;
		styleVarScope.Push(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		if (!ImGui::BeginChild(ImGui::GetID(this), ImVec2(boxSize.x, boxSize.y), childFlags, windowFlags))
		{
			ImGui::EndChild();
			return;
		}

		const bool hasVerticalScroll = IsScrollBarsVisible() && ImGui::GetScrollMaxY() > 0.0f;
		const bool hasHorizontalScroll = IsScrollBarsVisible() && IsHorizontalScrollBarEnabled() && ImGui::GetScrollMaxX() > 0.0f;
		const float scrollBarSize = ImGui::GetStyle().ScrollbarSize;

		// ---- MEASURE pass (children desired, with margins) ----
		struct ResolvedChild
		{
			IBaseWidget* pWidget		= nullptr;
			FloatRect Margin			= FloatRect::Zero();
			Vector2 Size				= Vector2::Zero;
			Vector2 OccupiedSize		= Vector2::Zero;
			bool StretchHorizontally	= false;
			bool StretchVertically		= false;
		};

		std::vector<ResolvedChild> list;
		list.reserve(m_Widgets.size());

		float totalFixedWidth = 0.0f;
		float rowMaxHeight = 0.0f;
		uint32 numWidgetsWithStretchPolicy = 0u;

		for (const Ref<IBaseWidget>& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible())
				continue;

			ResolvedChild& resolved = list.emplace_back();
			resolved.pWidget = pWidget.Get();
			resolved.Margin = pWidget->GetMargin();
			resolved.Size = pWidget->ReportSize();
			resolved.StretchHorizontally = pWidget->GetHorizontalSizePolicy() == ESizePolicy::Stretch;
			resolved.StretchVertically = pWidget->GetVerticalSizePolicy() == ESizePolicy::Stretch;
			resolved.OccupiedSize = Vector2(resolved.Size.x + resolved.Margin.Left + resolved.Margin.Right, resolved.Size.y + resolved.Margin.Top + resolved.Margin.Bottom);

			if (resolved.StretchHorizontally)
				numWidgetsWithStretchPolicy++;
			else
				totalFixedWidth += resolved.OccupiedSize.x;

			rowMaxHeight = Math::Max(rowMaxHeight, resolved.OccupiedSize.y);
		}

		const uint32 numWidgetsToRender = static_cast<uint32>(list.size());
		const float totalSpacing = (numWidgetsToRender > 0u) ? (m_Spacing.x * (numWidgetsToRender - 1u)) : 0.0f;
		const float totalStretchWidth = Math::Max(boxSize.x - totalFixedWidth - totalSpacing, 0.0f);
		const float stretchSlice = totalStretchWidth / Math::Max(numWidgetsWithStretchPolicy, 1u);
		const float totalPackedWidth = totalFixedWidth + totalSpacing;

		const EHorizontalAlignmentPolicy horizontalAlignmentPolicy = GetHorizontalAlignmentPolicy();
		
		float horizontalAlignmentOffsetX = 0.0f;
		if (horizontalAlignmentPolicy == EHorizontalAlignmentPolicy::Center)
			horizontalAlignmentOffsetX = Math::Max(0.0f, (boxSize.x - totalPackedWidth) * 0.5f);
		else if (horizontalAlignmentPolicy == EHorizontalAlignmentPolicy::Right)
			horizontalAlignmentOffsetX = Math::Max(0.0f, (boxSize.x - totalPackedWidth));
		
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + horizontalAlignmentOffsetX);

		// ---- RENDER pass ----
		ImVec2 origin = ImGui::GetCursorPos();
		ImVec2 layoutCursor = origin;
		
		const float rowSlotHeight = Math::Max(1.0f, boxSize.y);

		for (size_t i = 0; i < list.size(); ++i)
		{
			if (i != 0)
				layoutCursor.x += m_Spacing.x;

			ResolvedChild& child = list[i];
			
			float allocatedWidth = child.StretchHorizontally ? stretchSlice : child.OccupiedSize.x;
			const float allocatedHeight = rowSlotHeight;
			
			if (child.pWidget->GetHorizontalSizePolicy() == ESizePolicy::Auto)
			{
				const float remainingBoxWidth = Math::Max(0.0f, boxSize.x - layoutCursor.x);
				allocatedWidth = Math::Min(allocatedWidth, remainingBoxWidth);
			}
			
			const float contentWidth = Math::Max(0.0f, allocatedWidth - (child.Margin.Left + child.Margin.Right));
			const float contentHeight = Math::Max(0.0f, allocatedHeight - (child.Margin.Top + child.Margin.Bottom));
			
			float assignedWidth = contentWidth;
			float assignedHeight = contentHeight;

			const float desiredHeight = Math::Max(0.0f, child.Size.y);

			if (!child.pWidget->IsContainer() && !child.StretchVertically)
				assignedHeight = Math::Min(desiredHeight, contentHeight);

			const EVerticalAlignmentPolicy verticalAlignmentPolicy = child.pWidget->GetVerticalAlignmentPolicy();
			float alignmentYOffset = 0.0f;
			if (verticalAlignmentPolicy == EVerticalAlignmentPolicy::Center)
				alignmentYOffset = Math::Max(0.0f, (contentHeight - assignedHeight) * 0.5f);
			else if (verticalAlignmentPolicy == EVerticalAlignmentPolicy::Bottom)
				alignmentYOffset = Math::Max(0.0f, (contentHeight - assignedHeight));

			if (hasVerticalScroll && child.pWidget->GetHorizontalSizePolicy() != ESizePolicy::Fixed && i == list.size() - 1)
			{
				assignedWidth -= scrollBarSize;
				assignedWidth = Math::Max(0.0f, assignedWidth);
			}

			if (hasHorizontalScroll && child.pWidget->GetVerticalSizePolicy() != ESizePolicy::Fixed)
			{
				if (layoutCursor.y + assignedHeight + alignmentYOffset > boxSize.y - scrollBarSize)
					assignedHeight = Math::Max(0.0f, assignedHeight - scrollBarSize);
			}

			if (assignedWidth == 0.0f || assignedHeight == 0.0f)
				continue;

			ImGui::SetCursorPos(layoutCursor);

			ImVec2 contentPos = layoutCursor;
			contentPos.x += child.Margin.Left;
			contentPos.y += child.Margin.Top;
			contentPos.y += alignmentYOffset;

			ImGui::SetCursorPos(contentPos);

			const bool shouldAssignSize =
				child.pWidget->IsContainer() ||
				child.pWidget->RequiresAssignedSize();

			if (shouldAssignSize)
				child.pWidget->AssignSize({ assignedWidth, assignedHeight });

			if (child.pWidget->GetHorizontalSizePolicy() == ESizePolicy::Auto || child.pWidget->GetHorizontalSizePolicy() == ESizePolicy::Fixed || (!shouldAssignSize && child.pWidget->GetHorizontalSizePolicy() == ESizePolicy::Stretch))
				ImGui::SetNextItemWidth(assignedWidth);

			child.pWidget->Render();

			layoutCursor.x += allocatedWidth;
			layoutCursor.y = origin.y;
		}

		const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		const ImVec2 rMin = ImGui::GetWindowPos();
		const ImVec2 rMax = ImVec2(rMin.x + ImGui::GetWindowSize().x,
			rMin.y + ImGui::GetWindowSize().y);

		ImDrawList* parentDL = ImGui::GetCurrentWindow()->DrawList;

		ImGui::EndChild();

		if (hovered)
			parentDL->AddRect(rMin, rMax, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
	}

}