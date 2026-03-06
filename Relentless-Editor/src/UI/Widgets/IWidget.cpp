#include "IWidget.h"

namespace Relentless
{
	void IBaseWidget::AssignSize(const Vector2& aSize) noexcept
	{
		m_AssignedSize = aSize;
	}

	const Vector2& IBaseWidget::GetAssignedSize() const noexcept
	{
		return m_AssignedSize;
	}

	float IBaseWidget::GetFixedWidth() const noexcept
	{
		return m_FixedSize.x;
	}

	float IBaseWidget::GetFixedHeight() const noexcept
	{
		return m_FixedSize.y;
	}

	const Vector2& IBaseWidget::GetFixedSize() const noexcept
	{
		return m_FixedSize;
	}

	const FloatRect& IBaseWidget::GetMargin() const noexcept
	{
		return m_Margin;
	}

	EHorizontalAlignmentPolicy IBaseWidget::GetHorizontalAlignmentPolicy() const noexcept
	{
		return m_HorizontalAlignmentPolicy;
	}

	ESizePolicy IBaseWidget::GetHorizontalSizePolicy() const noexcept
	{
		return m_SizePolicy;
	}

	const FloatRect& IBaseWidget::GetPadding() const noexcept
	{
		return m_Padding;
	}

	ESizePolicy IBaseWidget::GetVerticalSizePolicy() const noexcept
	{
		return m_VerticalSizePolicy;
	}

	EVerticalAlignmentPolicy IBaseWidget::GetVerticalAlignmentPolicy() const noexcept
	{
		return m_VerticalAlignmentPolicy;
	}

	bool IBaseWidget::HasAssignedSize() const noexcept
	{
		return m_AssignedSize != Vector2::Zero;
	}

	bool IBaseWidget::IsEnabled() const noexcept
	{
		return m_IsEnabled;
	}

	bool IBaseWidget::IsVisible() const noexcept
	{
		return m_IsVisible;
	}

	IBaseWidget* IBaseWidget::SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy aAlignmentPolicy) noexcept
	{
		m_HorizontalAlignmentPolicy = aAlignmentPolicy;
		return this;
	}

	IBaseWidget* IBaseWidget::SetHorizontalSizePolicy(ESizePolicy aSizePolicy) noexcept
	{
		m_SizePolicy = aSizePolicy;
		return this;
	}

	void IBaseWidget::SetIsEnabled(bool aIsEnabledState) noexcept
	{
		if (m_IsEnabled == aIsEnabledState)
			return;

		m_IsEnabled = aIsEnabledState;
		OnEnabledStateChanged(m_IsEnabled);
	}

	void IBaseWidget::SetIsVisible(bool aVisibleState) noexcept
	{
		if (m_IsVisible == aVisibleState)
			return;

		m_IsVisible = aVisibleState;
		OnVisibilityChanged(m_IsVisible);
	}

	void IBaseWidget::SetMargin(const FloatRect& aMargin) noexcept
	{
		m_Margin = aMargin;
	}

	IBaseWidget* IBaseWidget::SetPadding(const FloatRect& aPadding) noexcept
	{
		m_Padding = aPadding;
		return this;
	}

	void IBaseWidget::SetSize(const Vector2& aSize) noexcept
	{
		m_FixedSize = aSize;
	}

	IBaseWidget* IBaseWidget::SetVerticalSizePolicy(ESizePolicy aSizePolicy) noexcept
	{
		m_VerticalSizePolicy = aSizePolicy;
		return this;
	}

	IBaseWidget* IBaseWidget::SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy aAlignmentPolicy) noexcept
	{
		m_VerticalAlignmentPolicy = aAlignmentPolicy;
		return this;
	}

	void WidgetStyle::Apply() noexcept
	{
		if (m_pFont)
			ImGui::PushFont(m_pFont);

		for (auto& [style, value] : m_Vars1)
			ImGui::PushStyleVar(style, value);

		for (auto& [style, value] : m_Vars2)
			ImGui::PushStyleVar(style, value);

		for (auto& [styleColor, value] : m_Cols)
			ImGui::PushStyleColor(styleColor, value);
	}

	void WidgetStyle::Discard() noexcept
	{
		ImGui::PopStyleColor(m_Cols.size());
		ImGui::PopStyleVar(m_Vars1.size() + m_Vars2.size());

		if (m_pFont)
			ImGui::PopFont();
	}

	ImFont* WidgetStyle::GetFont() const noexcept
	{
		return m_pFont;
	}

	Vector2 WidgetStyle::GetPadding() const noexcept
	{
		if (m_Vars1.contains(ImGuiStyleVar_FramePadding))
			return Vector2(m_Vars1.at(ImGuiStyleVar_FramePadding).x, m_Vars1.at(ImGuiStyleVar_FramePadding).y);

		return Vector2(ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y);
	}

	void WidgetStyle::SetFont(ImFont* pFont) noexcept
	{
		m_pFont = pFont;
	}

	void WidgetStyle::SetStyleVar(ImGuiStyleVar styleVar, ImVec2 value) noexcept
	{
		m_Vars1[styleVar] = value;
	}

	void WidgetStyle::SetStyleVar(ImGuiStyleVar styleVar, float value) noexcept
	{
		m_Vars2[styleVar] = value;
	}

	void WidgetStyle::SetStyleColor(ImGuiCol styleColor, ImVec4 value) noexcept
	{
		m_Cols[styleColor] = value;
	}

}