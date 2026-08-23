#include "DetailCategoryRow.h"

#include "Module/ModuleManager.h"
#include "Module/UIModule.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/Spacer.h"

namespace Relentless
{
	DetailCategoryRow::DetailCategoryRow(StringView aName, bool aIsExpanded) noexcept
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

		OnMouseDown.Connect([this](const WidgetGeometry&, const PointerInfo& aPointerInfo)
			{
				if (aPointerInfo.EffectingButton == RLS_Button::Right && m_OnContextMenuOpening.IsSet())
				{
					if (Ref<ContextMenu> pContextMenu = m_OnContextMenuOpening())
						ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(pContextMenu);
				}
			});
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

	void DetailCategoryRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		ImGuiTable* pTable = ImGui::GetCurrentTable();
		ImGuiWindow* pInnerWindow = pTable->InnerWindow;

		const ImVec2 clipMin(pTable->WorkRect.Min.x, Math::Max(pTable->WorkRect.Min.y, pInnerWindow->ClipRect.Min.y));
		const ImVec2 clipMax(pTable->WorkRect.Max.x, Math::Min(pTable->WorkRect.Max.y, pInnerWindow->ClipRect.Max.y));

		ImGui::PushClipRect(clipMin, clipMax, false);

		m_ColumnWidgets2[aColumn]->AssignSize(Vector2(pTable->WorkRect.GetWidth(), ReportSize().y));
		m_ColumnWidgets2[aColumn]->Render();

		ImGui::PopClipRect();
	}
}