#include "CollapsibleSection.h"

namespace Relentless
{
	CollapsibleSection::CollapsibleSection(std::string_view text) noexcept
		: m_Text{text}
	{
		SetFlags(ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_OpenOnArrow);
		SetSizePolicy(ESizePolicy::Stretch);
		
		const ImVec4& frameColor = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
		SetActiveColor(Color(frameColor.x, frameColor.y, frameColor.z, frameColor.w));
		SetHoverColor(Color(frameColor.x, frameColor.y, frameColor.z, frameColor.w));
		SetFont(ImGui::GetIO().Fonts->Fonts[2]);

		SetPadding(Vector2(5.0f, 6.0f));
	}

	float CollapsibleSection::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	bool CollapsibleSection::HasWidget(Ref<IWidget> pWidget) const noexcept
	{
		return std::find(m_Children.begin(), m_Children.end(), pWidget) != m_Children.end();
	}

	CollapsibleSection* CollapsibleSection::SetActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_HeaderActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	CollapsibleSection* CollapsibleSection::SetBackgroundColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Header, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	CollapsibleSection* CollapsibleSection::SetHoverColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	void CollapsibleSection::OnRender() noexcept
	{
		const bool isOpen = ImGui::CollapsingHeader(m_Text.c_str(), GetFlags());

		const float fullWidth = ImGui::GetContentRegionAvail().x;
		const float iconWidth = ImGui::CalcTextSize(ICON_FA_GEAR).x;
		constexpr float edgeOffset = 10.0f;

		ImGui::SameLine(fullWidth - iconWidth - edgeOffset);

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), ICON_FA_GEAR);

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
}