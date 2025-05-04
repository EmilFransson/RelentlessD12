#include "FloatSlider.h"

namespace Relentless
{

	FloatSlider::FloatSlider(std::string_view id, float startValue, float min, float max, const char* pFormat, int flags) noexcept
		:IWidget{ id }
		,m_Format{pFormat}
		,m_Value{ startValue }
		,m_Min{ min }
		,m_Max{ max }
	{
		SetFlags(flags);
	}

	float FloatSlider::GetValue() const noexcept
	{
		return m_Value;
	}

	void FloatSlider::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void FloatSlider::OnRender() noexcept
	{
		ImGui::GetWindowDrawList()->ChannelsSplit(2);

		auto curPos = ImGui::GetCursorScreenPos();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

			const float currentMinSize = ImGui::GetStyle().GrabMinSize;
			ImGui::GetStyle().GrabMinSize = 20.0f;

			SetColorsAndStyles();

			m_IsUsing = ImGui::SliderFloat(m_ID.c_str(), &m_Value, m_Min, m_Max, m_Format.c_str(), GetFlags());

			if (m_IsUsing)
				OnChanged(m_Value);

			m_IsHovered = ImGui::IsItemHovered();
			m_IsActive = ImGui::IsItemActive();

			DiscardAllStylesAndColors();
			ImGui::GetStyle().GrabMinSize = currentMinSize;
		}

		const ImVec2 size = ImGui::GetItemRectSize();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

			const ImVec2 min = ImVec2(curPos.x - 2, curPos.y - 2);
			const ImVec2 max = ImVec2(min.x + size.x + 4.0f, min.y + size.y + 4.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, m_IsActive ? IM_COL32(66, 150, 250, 255) : m_IsHovered ? IM_COL32(75, 75, 75, 255) : IM_COL32(50, 50, 50, 255), 6);
		}


		ImGui::GetWindowDrawList()->ChannelsMerge();
	}

	void FloatSlider::SetColorsAndStyles() noexcept
	{
		SetStyleColors
		({
			{ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_FrameBgHovered, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_FrameBgActive, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_SliderGrab, ImVec4(0.35f, 0.35f, 0.35f, 1.0f)}
		});

		SetStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
	}
}
