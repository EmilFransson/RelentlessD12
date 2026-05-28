#include "ContextMenuBuilder.h"

#include "ImGui/ImGuiFonts.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/SectionRow.h"
#include "UI/Widgets/Separator.h"
#include "UI/Widgets/SubMenuRow.h"

namespace Relentless
{
	TextItemBuilder::TextItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept
		: ContextMenuItemBuilder<TextItemBuilder>(aParent, aItem)
	{
	}

	TextItemBuilder& TextItemBuilder::OnClicked(Callback<void()>&& aCallback) noexcept
	{
		m_Item.OnClickedCallback = std::move(aCallback);
		return *this;
	}

	TextItemBuilder& TextItemBuilder::Shortcut(StringView aShortcut) noexcept
	{
		m_Item.Shortcut = aShortcut;
		return *this;
	}

	TextItemBuilder ContextMenuBuilder::AddItem(StringView aName) noexcept
	{
		ContextMenuItem& item = m_Items.emplace_back();
		item.Name = aName;
		return TextItemBuilder(*this, item);
	}

	TextItemBuilder ContextMenuBuilder::AddItem(StringView aName, Callback<void()>&& aCallback) noexcept
	{
		ContextMenuItem& item = m_Items.emplace_back();
		item.Type = ContextMenuItem::EType::TextItem;
		item.Name = aName;
		item.OnClickedCallback = std::move(aCallback);

		return TextItemBuilder(*this, item);
	}

	SectionItemBuilder ContextMenuBuilder::AddSection(StringView aText) noexcept
	{
		ContextMenuItem& item = m_Items.emplace_back();
		item.Type = ContextMenuItem::EType::Section;
		item.Name = aText;

		return SectionItemBuilder(*this, item);
	}

	SubmenuItemBuilder ContextMenuBuilder::AddSubmenu(StringView aName) noexcept
	{
		ContextMenuItem& item = m_Items.emplace_back();
		item.Type = ContextMenuItem::EType::Submenu;
		item.Name = aName;

		return SubmenuItemBuilder(*this, item);
	}

	SubmenuItemBuilder ContextMenuBuilder::AddSubmenu(StringView aName, Callback<void(ContextMenuBuilder&)>&& aCallback) noexcept
	{
		ContextMenuItem& item = m_Items.emplace_back();
		item.Type = ContextMenuItem::EType::Submenu;
		item.Name = aName;
		item.OnOpenSubmenuCallback = std::move(aCallback);

		return SubmenuItemBuilder(*this, item);
	}

	SeparatorItemBuilder ContextMenuBuilder::AddSeparator() noexcept
	{
		ContextMenuItem& item = m_Items.emplace_back();
		item.Type = ContextMenuItem::EType::Separator;

		return SeparatorItemBuilder(*this, item);
	}

	Ref<ContextMenu> ContextMenuBuilder::BuildContextMenu() noexcept
	{
		Ref<ContextMenu> pContextMenu = RLS_NEW ContextMenu();
		pContextMenu->AddRows(BuildRows());
		
		return pContextMenu;
	}

	Ref<ContextMenu> ContextMenuBuilder::BuildSubMenu() noexcept
	{
		Ref<ContextMenu> pSubMenu = RLS_NEW ContextMenu(true);
		pSubMenu->AddRows(BuildRows());
		
		return pSubMenu;
	}

	Ref<IBaseWidget> ContextMenuBuilder::BuildSectionRow(const ContextMenuItem& aItem) noexcept
	{
		Ref<SectionRow> pSection = RLS_NEW SectionRow(aItem.Name);
		pSection->SetSeparatorThickness(aItem.SeparatorThickness);

		if (aItem.SeparatorColor.IsSet())
			pSection->SetSeparatorColor(aItem.SeparatorColor.Get());
		if (aItem.TextColor.IsSet())
			pSection->SetTextColor(aItem.TextColor.Get());
		if (aItem.Font)
			pSection->SetFont(aItem.Font);

		return pSection;
	}

	Ref<IBaseWidget> ContextMenuBuilder::BuildSeparatorRow(const ContextMenuItem& aItem) noexcept
	{
		Ref<Separator> pSeparator = RLS_NEW Separator();
		pSeparator->SetThickness(aItem.SeparatorThickness);

		if (aItem.SeparatorColor.IsSet())
			pSeparator->SetActiveColor(aItem.SeparatorColor.Get());

		return pSeparator;
	}

	Ref<IBaseWidget> ContextMenuBuilder::BuildSubMenuRow(ContextMenuItem& aItem) noexcept
	{
		const String name = aItem.Icon.empty() ? aItem.Name : std::format("{} {}", aItem.Icon, aItem.Name);
		Ref<SubMenuRow> pSubMenuRow = RLS_NEW SubMenuRow(name);
		pSubMenuRow->OnOpen(std::move(aItem.OnOpenSubmenuCallback));
		pSubMenuRow->SetMargin(IntRect::Uniform(1));

		return pSubMenuRow;
	}

	Ref<IBaseWidget> ContextMenuBuilder::BuildTextRow(const ContextMenuItem& aItem) noexcept
	{
		Ref<HorizontalBox> pMainRow = RLS_NEW HorizontalBox();

		if (aItem.Enabled)
		{
			pMainRow->OnMouseEnter([](HorizontalBox* aRow) { aRow->SetBackgroundColor(Colors::RowFocusedSelectionColorDefault); });
			pMainRow->OnMouseExit([](HorizontalBox* aRow) { aRow->SetBackgroundColor(Colors::Transparent); });
		}

		pMainRow->SetMargin(IntRect::Uniform(1));
		pMainRow->SetPadding(FloatRect::WithLeft(5.0f));
		pMainRow->SetHoverFlags(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenOverlapped);
		
		if (!aItem.Enabled && !aItem.DisabledTooltip.empty())
			pMainRow->SetTooltipText(aItem.DisabledTooltip);
		else if (!aItem.Tooltip.empty())
			pMainRow->SetTooltipText(aItem.Tooltip);

		if (!aItem.Icon.empty())
		{
			HorizontalBox* pLeftBox = pMainRow->AddWidget(RLS_NEW HorizontalBox());
			pLeftBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
			pLeftBox->SetSize(Vector2(24.0f, 0.0f));
			pLeftBox->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);
			pLeftBox->AddWidget(RLS_NEW Label(aItem.Icon, aItem.Font))
				->SetIsEnabled(aItem.Enabled);
		}

		HorizontalBox* pTextBox = pMainRow->AddWidget(RLS_NEW HorizontalBox());
		pTextBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pTextBox->AddWidget(RLS_NEW Label(aItem.Name, aItem.Font))
			->SetIsEnabled(aItem.Enabled);

		if (!aItem.Shortcut.empty())
		{
			HorizontalBox* pRightBox = pMainRow->AddWidget(RLS_NEW HorizontalBox());
			pRightBox->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Right);
			pRightBox->AddWidget(RLS_NEW Label(aItem.Shortcut, aItem.Font))
				->SetTextColor(Colors::TextInactive);
		}

		if (aItem.Enabled && aItem.OnClickedCallback.IsSet())
		{
			pMainRow->OnMouseDown.Connect([callBack = std::move(aItem.OnClickedCallback)]
			(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo)
				{
					callBack();
				});
		}

		return pMainRow;
	}

	std::vector<Ref<IBaseWidget>> ContextMenuBuilder::BuildRows() noexcept
	{
		 std::vector<Ref<IBaseWidget>> rows;
		 rows.reserve(m_Items.size());

		 for (auto& item : m_Items)
		 {
			 switch (item.Type)
			 {
			 case ContextMenuItem::EType::TextItem: rows.push_back(BuildTextRow(item)); break;
			 case ContextMenuItem::EType::Submenu:  rows.push_back(BuildSubMenuRow(item)); break;
			 case ContextMenuItem::EType::Separator: rows.push_back(BuildSeparatorRow(item)); break;
			 case ContextMenuItem::EType::Section: rows.push_back(BuildSectionRow(item)); break;
			 default: RLS_ASSERT(false, "[ContextMenuBuilder::BuildRows]: Unknown context menu type encountered."); break;
			 }
		 }

		 return rows;
	}

	SectionItemBuilder::SectionItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept
		: ContextMenuItemBuilder<SectionItemBuilder>(aParent, aItem)
	{}

	SectionItemBuilder& SectionItemBuilder::SeparatorColor(const Color& aColor) noexcept
	{
		m_Item.SeparatorColor.Set(aColor);
		return *this;
	}

	SectionItemBuilder& SectionItemBuilder::Thickness(float aThickness) noexcept
	{
		m_Item.SeparatorThickness = aThickness;
		return *this;
	}

	SectionItemBuilder& SectionItemBuilder::TextColor(const Color& aColor) noexcept
	{
		m_Item.TextColor.Set(aColor);
		return *this;
	}

	SeparatorItemBuilder::SeparatorItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept
		: ContextMenuItemBuilder<SeparatorItemBuilder>(aParent, aItem)
	{}

	SeparatorItemBuilder& SeparatorItemBuilder::Color(const DirectX::SimpleMath::Color& aColor) noexcept
	{
		m_Item.SeparatorColor.Set(aColor);
		return *this;
	}

	SeparatorItemBuilder& SeparatorItemBuilder::Thickness(float aThickness) noexcept
	{
		m_Item.SeparatorThickness = aThickness;
		return *this;
	}

	SubmenuItemBuilder::SubmenuItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept
		: ContextMenuItemBuilder<SubmenuItemBuilder>(aParent, aItem)
	{
	}

	SubmenuItemBuilder& SubmenuItemBuilder::OnOpen(Callback<void(ContextMenuBuilder&)>&& aCallback) noexcept
	{
		m_Item.OnOpenSubmenuCallback = std::move(aCallback);
		return *this;
	}
}
