#include "Tooltip.h"

#include "ImGui/ImguiLayer.h"

namespace Relentless
{
	Tooltip::Tooltip(std::string_view text ) noexcept
		: m_Text{text}
	{
	}

	void Tooltip::OnRender() noexcept
	{
		if (m_Text.empty())
			return;

		ImGui::PushID((const void*)this);

		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		
		ImGui::BeginTooltip();

		ImGui::Text(m_Text.c_str());
		
		ImGui::EndTooltip();

		ImGui::PopStyleColor(2);

		ImGui::PopID();
	}

	const String& Tooltip::GetText() const noexcept
	{
		return m_Text;
	}

	void Tooltip::SetText(std::string_view text) noexcept
	{
		m_Text = text;
	}
}
