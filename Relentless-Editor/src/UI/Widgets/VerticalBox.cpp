#include "VerticalBox.h"

namespace Relentless
{
	VerticalBox::VerticalBox(const Vector2 aSize, bool aIsChildRegion) noexcept
		: IWidgetContainer<VerticalBox>(aSize, aIsChildRegion)
	{
	}

	Vector2 VerticalBox::ReportSize() const noexcept
	{
		if (GetHorizontalSizePolicy() == ESizePolicy::Fixed)
			return m_Size;

		float totalHeight = 0.0f;
		float maxWidth = 0.0f;
		uint32 visibleCount = 0u;

		for (const auto& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible())
				continue;

			const Vector2 size = pWidget->ReportSize();
			const FloatRect margin = pWidget->GetMargin();

			totalHeight += (size.y + margin.Top + margin.Bottom);
			maxWidth = Math::Max(maxWidth, size.x + margin.Left + margin.Right);

			visibleCount++;
		}

		if (visibleCount > 1)
			totalHeight += m_Spacing.y * (visibleCount - 1u);

		const FloatRect& padding = GetPadding();
		totalHeight += padding.Top + padding.Bottom;
		maxWidth += padding.Left + padding.Right;

		return Vector2(maxWidth, totalHeight);
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

		float sumFixedContentHeight = 0.0f;
		float sumMarginsY = 0.0f;
		int   numStretchY = 0;
		float maxRowWidth = 0.0f; // cross-axis

		for (const auto& pWidget : m_Widgets)
		{
			if (!pWidget->IsVisible()) 
				continue;

			ChildMeasure measuredChild
			{
				pWidget,
				pWidget->ReportSize(),
				pWidget->GetMargin(),
				(pWidget->GetHorizontalSizePolicy() == ESizePolicy::Stretch),
			};

			// primary axis (Y)
			sumMarginsY += (measuredChild.Margin.Top + measuredChild.Margin.Bottom);
			if (measuredChild.Stretch) 
				++numStretchY;
			else             
				sumFixedContentHeight += measuredChild.Size.y;

			// cross axis (X): width is max of content + margins (for alignment/bounds)
			maxRowWidth = Math::Max(maxRowWidth, measuredChild.Size.x + measuredChild.Margin.Left + measuredChild.Margin.Right);

			list.push_back(measuredChild);
		}

		const int   n = (int)list.size();
		const float spacingY = m_Spacing.y;
		const float totalSpacingY = (n > 0) ? spacingY * (n - 1) : 0.0f;

		// Inner available height at current cursor (child window already removes its own padding)
		const float innerAvailableHeight = Math::Max(0.0f, ImGui::GetContentRegionAvail().y - (m_IsChildRegion ? 0.0f : padding.Bottom));

		// Solve stretch slice for HEIGHT (content-only)
		const float stretchContentHeight = (innerAvailableHeight - sumFixedContentHeight - sumMarginsY - totalSpacingY) / Math::Max(numStretchY, 1);

		// Total occupied height (what alignment acts on)
		const float totalContentHeight = sumMarginsY + totalSpacingY + sumFixedContentHeight + Math::Max(0.0f, stretchContentHeight) * numStretchY;

		// ===== VERTICAL alignment (Top/Center/Bottom) =====
		float offsetY = 0.0f;
		switch (GetVerticalAlignmentPolicy())
		{
		case EVerticalAlignmentPolicy::Center: 
			offsetY = Math::Max(0.0f, (innerAvailableHeight - totalContentHeight) * 0.5f);
			break;
		case EVerticalAlignmentPolicy::Bottom: 
			offsetY = Math::Max(0.0f, (innerAvailableHeight - totalContentHeight)); 
			break;
		default:
			break;
		}
		if (offsetY > 0.0f)
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

		// Use one spacing mechanism: ItemSpacing.y
		ScopedStyleVar itemStyle;
		itemStyle.Push(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, spacingY));

		// ===== LAYOUT pass =====
		ImVec2 boundsMin(+FLT_MAX, +FLT_MAX), boundsMax(-FLT_MAX, -FLT_MAX);
		bool anyItem = false;

		ImGui::BeginGroup();

		bool first = true;
		for (const auto& visibleChild : list)
		{
			if (!first) {
				// rely on ItemSpacing.y to advance between rows
			}
			else {
				first = false;
			}

			// Apply TOP/LEFT margins via cursor offsets
			ImVec2 currentCursorPos = ImGui::GetCursorPos();
			ImGui::SetCursorPos({ currentCursorPos.x + visibleChild.Margin.Left, currentCursorPos.y + visibleChild.Margin.Top });

			// Compute content height (stretch or fixed)
			const float contentHeight = visibleChild.Stretch ? Math::Max(0.0f, stretchContentHeight) : visibleChild.Size.y;

			// CROSS-AXIS (X) alignment per box policy (optional “Fill” via stretchX)
			const float innerWidth = Math::Max(0.0f, ImGui::GetContentRegionAvail().x - (m_IsChildRegion ? 0.0f : padding.Right));

			const float contentWidth = /*cm.stretchX ? std::max(0.0f, innerW - (cm.mg.Left + cm.mg.Right)) :*/ visibleChild.Size.x;

			switch (GetHorizontalAlignmentPolicy()) // alignment inside VBox across X
			{
			case EHorizontalAlignmentPolicy::Left:
				// cursor already at left + margin
				break;
			case EHorizontalAlignmentPolicy::Center:
			{
				const float freeX = Math::Max(0.0f, innerWidth - (contentWidth + visibleChild.Margin.Left + visibleChild.Margin.Right));
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + freeX * 0.5f);
				break;
			}
			case EHorizontalAlignmentPolicy::Right:
			{
				const float freeX = Math::Max(0.0f, innerWidth - (contentWidth + visibleChild.Margin.Left + visibleChild.Margin.Right));
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + freeX);
				break;
			}
			}

			// Hints for typical ImGui inputs/sliders
			if (!visibleChild.pWidget->IsContainer())
				ImGui::SetNextItemWidth(contentWidth);

			// Container child? Give it the box it should use (still cursor-driven, no AssignedRect object)
			if (visibleChild.pWidget->IsContainer())
			{
				visibleChild.pWidget->AssignSize({ contentWidth, contentHeight }); // your virtual setter used by its own BeginChild
			}

			// Render
			visibleChild.pWidget->Render();
			anyItem = true;

			// Update outer bounds
			ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
			boundsMin.x = std::min(boundsMin.x, a.x); boundsMin.y = std::min(boundsMin.y, a.y);
			boundsMax.x = std::max(boundsMax.x, b.x); boundsMax.y = std::max(boundsMax.y, b.y);

			// Advance by RIGHT/BOTTOM margins without adding extra spacing twice:
			// - For VERTICAL flow we need to consume bottom margin by a zero-width dummy of that height.
			ImGui::Dummy(ImVec2(0.0f, visibleChild.Margin.Bottom));

			// For left/right margins, we already offset before render; no extra horizontal dummy needed here.
		}

		ImGui::EndGroup();

		// If VBox itself stretches vertically, consume remaining height so parent flow continues
		if (GetHorizontalSizePolicy() == ESizePolicy::Stretch && !HasAssignedSize())
		{
			const float toConsume = ImGui::GetContentRegionAvail().y;
			if (toConsume > 0.0f) 
				ImGui::Dummy(ImVec2(0.0f, toConsume));
		}

		// Hover union
		if (anyItem)
		{
			const bool hovered = ImGui::IsMouseHoveringRect(boundsMin, boundsMax);
			if (!m_IsHovered && hovered) 
				OnMouseEnter_private();
			else if (m_IsHovered && !hovered) 
				OnMouseExit_private();
		}

		// Close inner child region if we opened it
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