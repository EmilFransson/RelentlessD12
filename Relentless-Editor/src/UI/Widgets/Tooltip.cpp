#include "Tooltip.h"
#include "ImGui/ImGuiIncludes.h"

namespace Relentless
{
	Tooltip::Tooltip(StringView aText) noexcept
	{
		SetText(aText);
	}

	String Tooltip::GetText() const noexcept
	{
		return m_TooltipProvider.IsSet() ? m_TooltipProvider() : String();
	}

	void Tooltip::OnRender() noexcept
	{
		const String text = GetText();
		if (text.empty())
			return;

		ImGui::PushID((const void*)this);

		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
		ImGui::BeginTooltip();

		ImGui::Text("%s", text.c_str());
		
		ImGui::EndTooltip();

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();

		ImGui::PopID();
	}

	void Tooltip::SetText(StringView aText) noexcept
	{
		m_TooltipProvider = [text = String(aText)]() { return text; };
	}

	void Tooltip::SetText(Callback<String()>&& aTextCallback) noexcept
	{
		m_TooltipProvider = std::move(aTextCallback);
	}
}
