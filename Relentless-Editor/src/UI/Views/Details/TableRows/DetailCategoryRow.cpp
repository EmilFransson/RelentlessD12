#include "DetailCategoryRow.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/Label.h"

namespace Relentless
{
	DetailCategoryRow::DetailCategoryRow(std::string_view aName, bool aIsExpanded) noexcept
	{
		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		pBox->AddWidget(RLS_NEW Button(aIsExpanded ? ICON_FA_CHEVRON_DOWN : ICON_FA_CHEVRON_RIGHT, Vector2(25.0f, 30.0f)))
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Transparent)
			->SetBorderColor(Colors::Transparent)
			->SetTextColor(Colors::Gray)
			->SetFont(ImGui::GetIO().Fonts->Fonts[2]);

		pBox->AddWidget(RLS_NEW Label(aName))
			->SetFont(ImGui::GetIO().Fonts->Fonts[2]);

		m_ColumnWidgets2.push_back(pBox);
	}

	Button* DetailCategoryRow::GetExpandButton() const noexcept
	{
		return static_cast<HorizontalBox*>(m_ColumnWidgets2[0].Get())->GetWidget<Button>(0);
	}

	Vector2 DetailCategoryRow::ReportSize() const noexcept
	{
		return m_ColumnWidgets2[0]->ReportSize();
	}

	const Color& DetailCategoryRow::GetBackgroundColor() const noexcept
	{
		static constexpr Color bgColor = Colors::Normalize(47.0f, 47.07, 47.0f, 255.0f);
		return bgColor;
	}

	uint32 DetailCategoryRow::GetNumColumns() noexcept
	{
		return 1u;
	}

	bool DetailCategoryRow::IsDragDropEligible() noexcept
	{
		return false;
	}

	void DetailCategoryRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		ImGuiTable* pTable = ImGui::GetCurrentTable();
		const ImVec2 clipMin(pTable->WorkRect.Min.x, pTable->WorkRect.Min.y);
		const ImVec2 clipMax(clipMin.x + pTable->WorkRect.GetWidth(), clipMin.y + pTable->WorkRect.GetHeight());

		ImGui::PushClipRect(clipMin, clipMax, false);

		m_ColumnWidgets2[aColumn]->AssignSize(Vector2(pTable->WorkRect.GetWidth(), ReportSize().y));
		m_ColumnWidgets2[aColumn]->Render();

		ImGui::PopClipRect();
	}
}