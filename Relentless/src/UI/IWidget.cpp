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
}