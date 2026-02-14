#include "EntityDetailCategoryRow.h"

#include "UI/Widgets/Button.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"

namespace Relentless
{
	EntityDetailCategoryRow::EntityDetailCategoryRow(std::string_view aName, bool aIsExpanded) noexcept
	{
		Ref<HorizontalBox> pBox = new HorizontalBox();
		pBox->SetSpacing(1.0f);

		pBox->AddWidget(new Button(aIsExpanded ? ICON_FA_CHEVRON_DOWN : ICON_FA_CHEVRON_RIGHT, Vector2(25.0f, 30.0f)))
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Transparent)
			->SetBorderColor(Colors::Transparent)
			->SetTextColor(Colors::Gray)
			->SetFont(ImGui::GetIO().Fonts->Fonts[2]);

		pBox->AddWidget(new Label(aName));

		SetColumnWidget(0, pBox);
	}

	Button* EntityDetailCategoryRow::GetExpandButton() const noexcept
	{
		return m_ColumnWidgets[0]->GetWidget<Button>(0);
	}

	Vector2 EntityDetailCategoryRow::ReportSize() const noexcept
	{
		return m_ColumnWidgets[0]->ReportSize();
	}

	const Color& EntityDetailCategoryRow::GetBackgroundColor() const noexcept
	{
		static constexpr Color bgColor = Colors::Normalize(47.0f, 47.07, 47.0f, 255.0f);
		return bgColor;
	}

	uint32 EntityDetailCategoryRow::GetNumColumns() noexcept
	{
		return 1u;
	}

	bool EntityDetailCategoryRow::IsDragDropEligible() noexcept
	{
		return false;
	}

	void EntityDetailCategoryRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		m_ColumnWidgets[aColumn]->Render();
	}
}