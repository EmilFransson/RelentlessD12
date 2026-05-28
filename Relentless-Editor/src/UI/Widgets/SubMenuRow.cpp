#include "SubMenuRow.h"

#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Widgets/ContextMenu.h"

namespace Relentless
{
	SubMenuRow::SubMenuRow(StringView aText) noexcept
		: m_Text(aText)
	{
	}

	SubMenuRow* SubMenuRow::OnOpen(Callback<void(ContextMenuBuilder&)>&& aCallback) noexcept
	{
		m_OnOpenCallback = std::move(aCallback);
		return this;
	}

	void SubMenuRow::OnRender() noexcept
	{
		if (ImGui::BeginMenu(m_Text.c_str(), m_Enabled))
		{
			if (!m_pSubMenu)
			{
				ContextMenuBuilder builder;
				if (m_OnOpenCallback.IsSet())
					m_OnOpenCallback(builder);

				m_pSubMenu = builder.BuildSubMenu();
				RLS_ASSERT(m_pSubMenu, "[SubMenuRow::OnRender]: Failed to build sub-menu");
			}
			m_pSubMenu->Render();
			ImGui::EndMenu();
		}
		else
			m_pSubMenu.Reset();
	}

	Vector2 SubMenuRow::ReportSize() const noexcept
	{
		ImFont* pFont = m_Style.GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;
		const ImVec2 textSize = ImGui::CalcTextSize(m_Text.c_str());
		const float chevronWidth = ImGui::GetFontSize() + ImGui::GetStyle().ItemInnerSpacing.x;

		if (pFont)
			ImGui::PopFont();

		return Vector2(textSize.x + chevronWidth + padding.x, frameHeight);
	}
}