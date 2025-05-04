#include "CollapsibleSection.h"

namespace Relentless
{
	CollapsibleSection::CollapsibleSection(std::string_view id) noexcept
		:IWidget{id}
	{
		SetFlags(ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth);
	}

	void CollapsibleSection::Add(Ref<IWidget> pWidget) noexcept
	{
		RLS_ASSERT(!HasWidget(pWidget), "[CollapsibleSection::Add] Widget already assigned as child.");
		m_Children.push_back(pWidget);
	}

	bool CollapsibleSection::HasWidget(Ref<IWidget> pWidget) const noexcept
	{
		return std::find(m_Children.begin(), m_Children.end(), pWidget) != m_Children.end();
	}

	void CollapsibleSection::OnRender() noexcept
	{
		SetColorsAndStyles();
		const bool isOpen = ImGui::CollapsingHeader(m_ID.c_str(), GetFlags());
		DiscardAllStylesAndColors();
		
		DetermineOpenState(isOpen);

		if (m_IsOpen)
		{
			for (const auto& pChild : m_Children)
				pChild->Render();
		}
	}

	void CollapsibleSection::DetermineOpenState(bool isOpenThisFrame) noexcept
	{
		if (isOpenThisFrame && !m_IsOpen)
			OnOpenStateChanged(true);
		else if (!isOpenThisFrame && m_IsOpen)
			OnOpenStateChanged(false);

		m_IsOpen = isOpenThisFrame;
	}

	void CollapsibleSection::SetColorsAndStyles() noexcept
	{
		SetStyleVars
		({
			{ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)},
			{ImGuiStyleVar_FramePadding, ImVec2(5.0f, 8.0f)},
			{ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)}
		});

		SetStyleColors
		({
			{ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg)},
			{ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg)}
		});
	}

}