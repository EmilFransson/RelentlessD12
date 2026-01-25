#include "Separator.h"

namespace Relentless
{
	Separator::Separator(const String& label, float spacing) noexcept
		: m_Label{ label }, m_Spacing{ spacing }
	{
	}

	float Separator::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	void Separator::OnRender() noexcept
	{
		ImGui::AlignTextToFramePadding();
		ImGui::SetWindowFontScale(0.85f);
		ImGui::TextUnformatted(m_Label.c_str());      
		ImGui::SameLine(0.0f, m_Spacing);

		// how much room we have for the line
		float avail = ImGui::GetContentRegionAvail().x - GetMargin().Right;
		if (avail <= 0.0f)
			return;

		// where in screen space we start
		ImVec2 p0 = ImGui::GetCursorScreenPos();

		// vertically center the line on the text
		float y = p0.y + ImGui::GetTextLineHeightWithSpacing() * 0.5f;

		// draw the line out to the right
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(p0.x, y),
			ImVec2(p0.x + avail, y),
			ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.6f))
		);
		ImGui::SetWindowFontScale(1.0f);

		//ImGui::Dummy({ avail + ImGui::CalcTextSize(m_Label.c_str()).x + m_Spacing, 0 });
	}
}