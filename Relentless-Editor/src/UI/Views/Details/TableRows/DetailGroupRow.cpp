#include "DetailGroupRow.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/Label.h"

namespace Relentless
{
	DetailGroupRow::DetailGroupRow(StringView aName, bool aIsExpanded) noexcept
	{
		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		pBox->AddWidget(RLS_NEW Button(aIsExpanded ? ICON_FA_CHEVRON_DOWN : ICON_FA_CHEVRON_RIGHT, Vector2(25.0f, 30.0f)))
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Transparent)
			->SetBorderColor(Colors::Transparent)
			->SetTextColor(Colors::Gray);

		pBox->AddWidget(RLS_NEW Label(aName));

		m_ColumnWidgets2.push_back(pBox);
	}

	Button* DetailGroupRow::GetExpandButton() const noexcept
	{
		return static_cast<HorizontalBox*>(m_ColumnWidgets2[0].Get())->GetWidget<Button>(0);
	}

	Vector2 DetailGroupRow::ReportSize() const noexcept
	{
		return m_ColumnWidgets2.front()->ReportSize();
	}

	const Color& DetailGroupRow::GetBackgroundColor() const noexcept
	{
		return Colors::Transparent;
	}

	uint32 DetailGroupRow::GetNumColumns() noexcept
	{
		return 1u;
	}

	void DetailGroupRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		if (aColumn == 0u)
		{
			for (uint32 i = 1u; i < m_IndentationLevel; ++i)
				ImGui::Indent();
		}

		m_ColumnWidgets2[aColumn]->AssignSize({ ImGui::GetContentRegionAvail().x, ReportSize().y });
		m_ColumnWidgets2[aColumn]->Render();

		if (aColumn == 0u)
		{
			for (uint32 i = 1u; i < m_IndentationLevel; ++i)
				ImGui::Unindent();
		}
	}
}