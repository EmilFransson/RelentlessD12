#include "IWidget.h"

namespace Relentless
{
	IWidget::IWidget(std::string_view id) noexcept
		: m_ID{id}
	{
	}

	void IWidget::AddFlags(int flags) noexcept
	{
		m_Flags |= flags;
	}

	void IWidget::AddSearchTags(Span<String> searchTags) noexcept
	{
		for (auto& searchTag : searchTags)
			m_SearchTags.insert(searchTag);
	}

	int IWidget::GetFlags() const noexcept
	{
		return m_Flags;
	}

	ESizePolicy IWidget::GetSizePolicy() const noexcept
	{
		return m_SizePolicy;
	}

	bool IWidget::HasFlags(int flags) const noexcept
	{
		return (m_Flags & flags) == flags;
	}

	bool IWidget::HasSearchTag(const String& searchTag) const noexcept
	{
		return m_SearchTags.contains(searchTag);
	}

	void IWidget::Render() noexcept
	{
		ImGui::PushID(this);

		if (!IsEnabled())
			ImGui::BeginDisabled();

		OnPreRender();
		OnRender();
		OnPostRender();
	
		if (!IsEnabled())
			ImGui::EndDisabled();

		ImGui::PopID();
	}

	void IWidget::SetFlags(int flags) noexcept
	{
		m_Flags = flags;
	}

	void IWidget::DiscardAllStylesAndColors()
	{
		ImGui::PopStyleVar(m_NumStyleVars);
		ImGui::PopStyleColor(m_NumStyleColors);

		m_NumStyleVars = m_NumStyleColors = 0;
	}

	bool IWidget::IsEnabled() const noexcept
	{
		return m_IsEnabled;
	}

	void IWidget::SetIsEnabled(bool state) noexcept
	{
		if (m_IsEnabled == state)
			return;

		m_IsEnabled = state;
		OnEnabledStateChanged(m_IsEnabled);
	}

	void IWidget::SetSizePolicy(ESizePolicy sizePolicy) noexcept
	{
		m_SizePolicy = sizePolicy;
	}

	void IWidget::SetWidthConstraint(float width) noexcept
	{
		m_WidthConstraint = width;
	}

	void IWidget::SetStyleColors(Span<std::pair<ImGuiCol, ImVec4>> colors) noexcept
	{
		m_NumStyleColors += colors.GetSize();

		for (const auto& pair : colors)
			ImGui::PushStyleColor(pair.first, pair.second);
	}

	void IWidget::SetStyleColors(Span<std::pair<ImGuiCol, uint32>> colors) noexcept
	{
		m_NumStyleColors += colors.GetSize();

		for (const auto& pair : colors)
			ImGui::PushStyleColor(pair.first, pair.second);
	}

	void IWidget::SetStyleVar(ImGuiStyleVar styleVar, ImVec2 value) noexcept
	{
		ImGui::PushStyleVar(styleVar, value);
		m_NumStyleVars++;
	}

	void IWidget::SetStyleVar(ImGuiStyleVar styleVar, float value) noexcept
	{
		ImGui::PushStyleVar(styleVar, value);
		m_NumStyleVars++;
	}

	void IWidget::SetStyleVars(Span<std::pair<ImGuiStyleVar, ImVec2>> styles) noexcept
	{
		m_NumStyleVars += styles.GetSize();

		for (const auto& pair : styles)
			ImGui::PushStyleVar(pair.first, pair.second);
	}

	void IWidget::SetStyleVars(Span<std::pair<ImGuiStyleVar, float>> styles) noexcept
	{
		m_NumStyleVars += styles.GetSize();

		for (const auto& pair : styles)
			ImGui::PushStyleVar(pair.first, pair.second);
	}

	void WidgetStyle::Apply() noexcept
	{
		const ImVec2 pos = ImGui::GetCursorPos();
		if (m_Margin.Left > 0.0f) 
			ImGui::SetCursorPosX(pos.x + m_Margin.Left);
		if (m_Margin.Top > 0.0f) 
			ImGui::SetCursorPosY(pos.y + m_Margin.Top);

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

		const ImVec2 pos = ImGui::GetCursorPos();
		if (m_Margin.Right > 0.0f)
			ImGui::SetCursorPosX(pos.x + m_Margin.Right);
		if (m_Margin.Bottom > 0.0f)
			ImGui::SetCursorPosY(pos.y + m_Margin.Bottom);
	}

	void WidgetStyle::SetFont(ImFont* pFont) noexcept
	{
		m_pFont = pFont;
	}

	void WidgetStyle::SetMargin(const IntRect& margin) noexcept
	{
		m_Margin = margin;
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

	IStylableWidget::IStylableWidget(std::string_view id) noexcept
		: IWidget{id}
	{
	}

	void IStylableWidget::Render() noexcept
	{
		ImGui::PushID(this);

		if (!IsEnabled())
			ImGui::BeginDisabled();

		m_Style.Apply();

		OnPreRender();
		OnRender();
		OnPostRender();

		m_Style.Discard();

		if (!IsEnabled())
			ImGui::EndDisabled();

		ImGui::PopID();
	}

	void IStylableWidget::SetActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_FrameBgActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void IStylableWidget::SetAlpha(float alpha) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_Alpha, alpha);
	}

	void IStylableWidget::SetBackgroundColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_FrameBg, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void IStylableWidget::SetBorderColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Border, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void IStylableWidget::SetBorderSize(float size) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_FrameBorderSize, size);
	}

	void IStylableWidget::SetFont(ImFont* pFont) noexcept
	{
		m_Style.SetFont(pFont);
	}

	void IStylableWidget::SetFrameRounding(float rounding) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_FrameRounding, rounding);
	}

	void IStylableWidget::SetHoverColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_FrameBgHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void IStylableWidget::SetInnerSpacing(const Vector2& innerSpacing) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(innerSpacing.x,innerSpacing.y));
	}

	void IStylableWidget::SetMargin(const IntRect& margin) noexcept
	{
		m_Style.SetMargin(margin);
	}

	void IStylableWidget::SetPadding(const Vector2& padding) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding.x, padding.y));
	}

	void IStylableWidget::SetSpacing(const Vector2& spacing) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing.x, spacing.y));
	}

	void IStylableWidget::SetTextColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Text, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}
}