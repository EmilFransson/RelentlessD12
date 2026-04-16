#include "ContextMenu.h"

#include "SearchBar.h"
#include "Separator.h"

namespace Relentless
{
	MenuBuilder::MenuBuilder() noexcept
		: m_pMenu{ new ContextMenu() }
	{
		Ref<HorizontalBox> pBox = new HorizontalBox();
		pBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pBox->SetMargin(FloatRect(20, 10, 20, 0));
		
		pBox->AddWidget(new SearchBar("Start typing to search"))
			->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		
		Ref<MenuItem> pEntry = new MenuItem();
		pEntry->pWidget = pBox;
		pEntry->HighlightOnHover = false;
		
		m_pMenu->AddEntry(pEntry);
	}

	MenuBuilder* MenuBuilder::AddWidget(Ref<IBaseWidget> pWidget, const String& label, const String& tooltip) noexcept
	{
		Ref<HorizontalBox> pBox = new HorizontalBox();
		pBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pBox->AddWidget(pWidget);

		Ref<Label> pLabel = new Label(label, ImGui::GetIO().Fonts->Fonts[2]);
		pBox->AddWidget(pLabel);

		if (!tooltip.empty())
			pBox->SetTooltipText(tooltip);

		pBox->SetMargin(IntRect(40, 0, 0, 0));

		Ref<MenuItem> pItem = new MenuItem();
		pItem->pWidget = std::move(pBox);

		if (!m_SubMenuStack.empty())
			m_SubMenuStack.top()->Entries.push_back(pItem);
		else
			m_pMenu->AddEntry(pItem);

		return this;
	}

	Ref<ContextMenu> MenuBuilder::Build() noexcept
	{
		return m_pMenu;
	}

	MenuBuilder* MenuBuilder::AddSection(const String& label) noexcept
	{
		Ref<MenuItem> pItem = new MenuItem();

		Ref<HorizontalBox> pBox = new HorizontalBox();
		pBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pBox->SetMargin(FloatRect(20, 0, 20, 0));

		Ref<Separator> pSeparator = new Separator(label, 10.0f);
		pSeparator->SetMargin({ 0, 0, 20, 0 });
		pSeparator->SetAlpha(0.5f);
		pBox->AddWidget(pSeparator);

		pItem->pWidget = pBox;
		pItem->HighlightOnHover = false;

		m_pMenu->AddEntry(std::move(pItem));

		return this;
	}

	MenuBuilder* MenuBuilder::BeginSubMenu(const String& label, const String& tooltip, const String& icon) noexcept
	{
		Ref<HorizontalBox> pBox = new HorizontalBox();
		pBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pBox->SetMargin(FloatRect(0.0f, 4.0f, 0.0f, 0.0f));

		if (!icon.empty())
		{
			Ref<Label> pIconLabel = new Label(icon);
			pIconLabel->SetAlpha(0.5f);
			pBox->AddWidget(pIconLabel);
		}

		Ref<Label> pLabel = new Label(label);

		pBox->AddWidget(pLabel);

		Ref<Label> pChevronIcon = new Label(ICON_FA_CHEVRON_RIGHT);

		Ref<HorizontalBox> pRightBox = new HorizontalBox();
		pRightBox->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Right);
		pRightBox->SetMargin(FloatRect(0.0f, 0.0f, 20.0f, 0.0f));
		pRightBox->AddWidget(pChevronIcon);

		pBox->AddWidget(pRightBox);

		if (!tooltip.empty())
			pBox->SetTooltipText(tooltip);

		pBox->SetMargin(IntRect(40.0f, 0.0f, 0.0f, 0.0f));

		Ref<SubMenu> pItem = new SubMenu(m_SubMenuStack.size() + 1);
		pItem->pWidget = std::move(pBox);

		m_SubMenuStack.push(pItem);

		return this;
	}

	MenuBuilder* MenuBuilder::EndSubMenu() noexcept
	{
		Ref<SubMenu> pSubMenu = m_SubMenuStack.top();
		m_SubMenuStack.pop();

		if (m_SubMenuStack.empty())
			m_pMenu->AddEntry(pSubMenu);
		else
			m_SubMenuStack.top()->Entries.push_back(pSubMenu);

		return this;
	}

}
