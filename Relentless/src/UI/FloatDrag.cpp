#include "FloatDrag.h"
#include "Button.h"

namespace Relentless
{
	FloatDrag::FloatDrag(std::string_view id, float speed, float min, float max, const char* pFormat, int flags) noexcept
		:IWidget{ id }
		, m_Speed{ speed }
		, m_Min{ min }
		, m_Max{ max }
		, m_Format{ pFormat }
	{
		SetFlags(flags);
	}

	void FloatDrag::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void FloatDrag::OnRender() noexcept
	{
		ImGui::GetWindowDrawList()->ChannelsSplit(2);

		auto curPos = ImGui::GetCursorScreenPos();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

			const float currentMinSize = ImGui::GetStyle().GrabMinSize;
			ImGui::GetStyle().GrabMinSize = 20.0f;

			SetColorsAndStyles();

			float value = m_ValueCallback();
			m_IsUsing = ImGui::DragFloat(m_ID.c_str(), &value, m_Speed, m_Min, m_Max, m_Format.c_str(), GetFlags());

			if (m_IsUsing)
				m_OnChanged(value);

			m_IsHovered = ImGui::IsItemHovered();
			const bool isActive = ImGui::IsItemActive();

			if (isActive && !m_IsActive)
				OnActiveChanged(true);
			else if (!isActive && m_IsActive)
				OnActiveChanged(false);

			m_IsActive = isActive;

			DiscardAllStylesAndColors();
			ImGui::GetStyle().GrabMinSize = currentMinSize;
		}

		const ImVec2 size = ImGui::GetItemRectSize();

		if (m_DrawColorIndicator)
		{
			const ImVec2 indicatorStartLocation = ImVec2(curPos.x + 6.0f, curPos.y + 6.0f);
			ImGui::SetCursorScreenPos(indicatorStartLocation);

			const ImVec2 min = indicatorStartLocation;
			const ImVec2 max = ImVec2(min.x + 5.0f, min.y + size.y - 10.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(m_IndicatorColor.R(), m_IndicatorColor.G(), m_IndicatorColor.B(), m_IndicatorColor.A())), 3.0f);
		
			ImGui::SetCursorScreenPos(curPos);
		}

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

			const ImVec2 min = ImVec2(curPos.x - 2, curPos.y - 2);
			const ImVec2 max = ImVec2(min.x + size.x + 4.0f, min.y + size.y + 4.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, m_IsActive ? IM_COL32(66, 150, 250, 255) : m_IsHovered ? IM_COL32(75, 75, 75, 255) : IM_COL32(50, 50, 50, 255), 6);
		}


		ImGui::GetWindowDrawList()->ChannelsMerge();
	}

	void FloatDrag::SetDrawColorIndicator(bool state) noexcept
	{
		m_DrawColorIndicator = state;
	}

	void FloatDrag::SetIndicatorColor(const Color& color) noexcept
	{
		SetDrawColorIndicator(true);
		m_IndicatorColor = color;
	}

	void FloatDrag::SetColorsAndStyles() noexcept
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
