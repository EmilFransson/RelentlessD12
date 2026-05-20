#include "VerticalBox.h"

namespace Relentless
{
	Vector2 VerticalBox::ReportSize() const noexcept
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

		float totalHeight = 0.0f;
		float maxWidth = 0.0f;
		uint32 visibleCount = 0u;

		for (const auto& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible())
				continue;

			const Vector2 widgetSize = pWidget->ReportSize();
			const FloatRect margin = pWidget->GetMargin();

			totalHeight += (widgetSize.y + margin.Top + margin.Bottom);
			maxWidth = Math::Max(maxWidth, widgetSize.x + margin.Left + margin.Right);

			visibleCount++;
		}

		if (visibleCount > 1)
			totalHeight += m_Spacing.y * (visibleCount - 1u);

		const FloatRect& padding = GetPadding();
		totalHeight += padding.Top + padding.Bottom;
		maxWidth += padding.Left + padding.Right;

		if (!isFixedWidth)
			size.x = maxWidth;
		if (!isFixedHeight)
			size.y = totalHeight;

		return size;
	}

	VerticalBox* VerticalBox::SetSpacing(float aSpacing) noexcept
	{
		m_Spacing = Vector2(0.0f, aSpacing);
		return this;
	}

	void VerticalBox::OnRender() noexcept
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
			IBaseWidget* pWidget = nullptr;
			FloatRect Margin = FloatRect::Zero();
			Vector2 Size = Vector2::Zero;
			Vector2 OccupiedSize = Vector2::Zero;
			bool Stretch = false;
		};

		std::vector<ResolvedChild> list;
		list.reserve(m_Widgets.size());

		float totalFixedHeight = 0.0f;
		float rowMaxWidth = 0.0f;
		uint32 numWidgetsWithStretchPolicy = 0u;

		for (const Ref<IBaseWidget>& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible())
				continue;

			ResolvedChild& resolved = list.emplace_back();
			resolved.pWidget = pWidget.Get();
			resolved.Margin = pWidget->GetMargin();
			resolved.Size = pWidget->ReportSize();
			resolved.Stretch = pWidget->GetVerticalSizePolicy() == ESizePolicy::Stretch;
			resolved.OccupiedSize = Vector2(resolved.Size.x + resolved.Margin.Left + resolved.Margin.Right, resolved.Size.y + resolved.Margin.Top + resolved.Margin.Bottom);

			if (resolved.Stretch)
				numWidgetsWithStretchPolicy++;
			else
				totalFixedHeight += resolved.OccupiedSize.y;

			rowMaxWidth = Math::Max(rowMaxWidth, resolved.OccupiedSize.x);
		}

		const uint32 numWidgetsToRender = static_cast<uint32>(list.size());
		const float totalSpacing = (numWidgetsToRender > 0u) ? (m_Spacing.y * (numWidgetsToRender - 1u)) : 0.0f;
		const float totalStretchHeight = Math::Max(boxSize.y - totalFixedHeight - totalSpacing, 0.0f);
		const float stretchSlice = totalStretchHeight / Math::Max(numWidgetsWithStretchPolicy, 1u);
		const float totalPackedHeight = totalFixedHeight + totalSpacing;

		const EVerticalAlignmentPolicy verticalAlignmentPolicy = GetVerticalAlignmentPolicy();
		EHorizontalAlignmentPolicy horizontalAlignmentPolicy = GetHorizontalAlignmentPolicy();

		float verticalAlignmentOffsetY = 0.0f;
		if (verticalAlignmentPolicy == EVerticalAlignmentPolicy::Center)
			verticalAlignmentOffsetY = Math::Max(0.0f, (boxSize.y - totalPackedHeight) * 0.5f);
		else if (verticalAlignmentPolicy == EVerticalAlignmentPolicy::Bottom)
			verticalAlignmentOffsetY = Math::Max(0.0f, (boxSize.y - totalPackedHeight));

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + verticalAlignmentOffsetY);

		// ---- RENDER pass ----
		ImVec2 origin = ImGui::GetCursorPos();
		ImVec2 layoutCursor = origin;

		const float rowSlotWidth = Math::Max(1.0f, boxSize.x);

		for (size_t i = 0; i < list.size(); ++i)
		{
			if (i != 0)
				layoutCursor.y += m_Spacing.y;

			ResolvedChild& child = list[i];

			float allocatedHeight = child.Stretch ? stretchSlice : child.OccupiedSize.y;
			const float allocatedWidth = rowSlotWidth;

			if (child.pWidget->GetVerticalSizePolicy() == ESizePolicy::Auto)
			{
				const float remainingBoxHeight = Math::Max(0.0f, boxSize.y - layoutCursor.y);
				allocatedHeight = Math::Min(allocatedHeight, remainingBoxHeight);
			}

			const float contentWidth = Math::Max(0.0f, allocatedWidth - (child.Margin.Left + child.Margin.Right));
			const float contentHeight = Math::Max(0.0f, allocatedHeight - (child.Margin.Top + child.Margin.Bottom));

			float assignedWidth = contentWidth;
			float assignedHeight = contentHeight;
			const bool childWantsHorizontalStretch = (child.pWidget->GetHorizontalSizePolicy() == ESizePolicy::Stretch);

			const float desiredWidth = Math::Max(0.0f, child.Size.x);

			if (!child.pWidget->IsContainer() && !childWantsHorizontalStretch)
				assignedWidth = Math::Min(desiredWidth, contentWidth);

			horizontalAlignmentPolicy = child.pWidget->GetHorizontalAlignmentPolicy();

			float alignmentXOffset = 0.0f;
			if (horizontalAlignmentPolicy == EHorizontalAlignmentPolicy::Center)
				alignmentXOffset = Math::Max(0.0f, (contentWidth - assignedWidth) * 0.5f);
			else if (horizontalAlignmentPolicy == EHorizontalAlignmentPolicy::Right)
				alignmentXOffset = Math::Max(0.0f, (contentWidth - assignedWidth));

			if (hasVerticalScroll && child.pWidget->GetHorizontalSizePolicy() != ESizePolicy::Fixed)
			{
				if (layoutCursor.x + assignedWidth + alignmentXOffset > boxSize.x - scrollBarSize)
				{
					assignedWidth -= scrollBarSize;
					assignedWidth = Math::Max(0.0f, assignedWidth);
				}
			}
			if (hasHorizontalScroll && child.pWidget->GetHorizontalSizePolicy() != ESizePolicy::Fixed && i == list.size() - 1)
			{
				if (layoutCursor.y + assignedHeight > boxSize.y - scrollBarSize)
					assignedHeight = Math::Max(0.0f, assignedHeight - scrollBarSize);
			}

			if (assignedWidth <= 3.0f || assignedHeight <= 3.0f)
				continue;

			ImGui::SetCursorPos(layoutCursor);

			ImVec2 contentPos = layoutCursor;
			contentPos.x += alignmentXOffset;
			contentPos.x += child.Margin.Left;
			contentPos.y += child.Margin.Top;

			ImGui::SetCursorPos(contentPos);

			const bool shouldAssignSize =
				child.pWidget->IsContainer() ||
				child.pWidget->RequiresAssignedSize() ||
				child.Stretch;

			if (shouldAssignSize)
				child.pWidget->AssignSize({ assignedWidth, assignedHeight });

			if (child.pWidget->GetHorizontalSizePolicy() == ESizePolicy::Auto || (!shouldAssignSize && child.pWidget->GetHorizontalSizePolicy() == ESizePolicy::Stretch))
				ImGui::SetNextItemWidth(assignedWidth);

			child.pWidget->Render();

			layoutCursor.y += allocatedHeight;
			layoutCursor.x = origin.x;
		}

		const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		if (!m_IsHovered && hovered)
			OnMouseEnter_private();
		else if (m_IsHovered && !hovered)
			OnMouseExit_private();

		const bool isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		if (m_IsFocused != isFocused)
		{
			m_IsFocused = isFocused;
			OnFocusChanged(m_IsFocused);
		}

		//const ImVec2 rMin = ImGui::GetWindowPos();
		//const ImVec2 rMax = ImVec2(rMin.x + ImGui::GetWindowSize().x,
		//	rMin.y + ImGui::GetWindowSize().y);

		//ImDrawList* parentDL = ImGui::GetCurrentWindow()->DrawList;

		ImGui::EndChild();

		//if (hovered)
		//	parentDL->AddRect(rMin, rMax, IM_COL32(255, 0, 255, 255), 0.0f, 0, 2.0f);
	}
}