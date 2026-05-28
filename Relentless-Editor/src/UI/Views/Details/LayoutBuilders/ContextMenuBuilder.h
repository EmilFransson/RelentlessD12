#pragma once
#include <Relentless.h>

#include "UI/Widgets/ContextMenu.h"
#include "UI/Widgets/IBaseWidget.h"

namespace Relentless
{
	class ContextMenuBuilder;

	struct ColorEntry
	{
		NO_DISCARD const Color& Get() const noexcept { return m_Color; };

		NO_DISCARD bool IsSet() const noexcept { return m_IsSet; }

		void Set(const Color& aColor) noexcept
		{
			m_Color = aColor;
			m_IsSet = true;
		}

	private:
		Color m_Color;
		bool m_IsSet = false;
	};

	struct ContextMenuItem
	{
		enum class EType : uint8 { TextItem = 0u, Checkbox, Submenu, Separator, Section, Custom };

		String Name;
		String Tooltip;
		String DisabledTooltip;
		String Icon;
		String Shortcut;
		ColorEntry SeparatorColor;
		ColorEntry TextColor;
		Callback<void()> OnClickedCallback;
		Callback<void(ContextMenuBuilder&)> OnOpenSubmenuCallback;
		float SeparatorThickness = 1.0f;
		ImFont* Font = nullptr;
		EType Type = EType::TextItem;
		bool Enabled = true;
	};

	template<typename Derived>
	class ContextMenuItemBuilder
	{
	public:
		explicit ContextMenuItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept;
		
		Derived& DisabledTooltip(StringView aText) noexcept;
		ContextMenuBuilder& Done() noexcept;

		Derived& Enabled(bool aState) noexcept;
		
		Derived& Font(ImFont* aFont) noexcept;

		Derived& Icon(StringView aIcon) noexcept;
		
		Derived& Tooltip(StringView aText) noexcept;
	protected:
		Derived& Self() noexcept;
		
		ContextMenuBuilder& m_Parent;
		ContextMenuItem& m_Item;
	};

	template<typename Derived>
	Derived& ContextMenuItemBuilder<Derived>::DisabledTooltip(StringView aText) noexcept
	{
		m_Item.DisabledTooltip = aText;
		return Self();
	}

	template<typename Derived>
	ContextMenuBuilder& ContextMenuItemBuilder<Derived>::Done() noexcept
	{
		return m_Parent;
	}

	template<typename Derived>
	Derived& ContextMenuItemBuilder<Derived>::Font(ImFont* aFont) noexcept
	{
		m_Item.Font = aFont;
		return Self();
	}

	template<typename Derived>
	Derived& ContextMenuItemBuilder<Derived>::Enabled(bool aState) noexcept
	{
		m_Item.Enabled = aState; 
		return Self();
	}

	template<typename Derived>
	Derived& ContextMenuItemBuilder<Derived>::Icon(StringView aIcon) noexcept
	{
		m_Item.Icon = aIcon;
		return Self();
	}

	template<typename Derived>
	Derived& ContextMenuItemBuilder<Derived>::Self() noexcept
	{
		return static_cast<Derived&>(*this);
	}

	template<typename Derived>
	Derived& ContextMenuItemBuilder<Derived>::Tooltip(StringView aText) noexcept
	{
		m_Item.Tooltip = aText; 
		return Self();
	}

	template<typename Derived>
	ContextMenuItemBuilder<Derived>::ContextMenuItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept
		:m_Parent(aParent),
		 m_Item(aItem)
	{
	}

	class TextItemBuilder : public ContextMenuItemBuilder<TextItemBuilder>
	{
	public:
		explicit TextItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept;

		TextItemBuilder& OnClicked(Callback<void()>&& aCallback) noexcept;

		TextItemBuilder& Shortcut(StringView aShortcut) noexcept;
	};

	class SectionItemBuilder : public ContextMenuItemBuilder<SectionItemBuilder>
	{
	public:
		explicit SectionItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept;

		SectionItemBuilder& SeparatorColor(const Color& aColor) noexcept;

		SectionItemBuilder& Thickness(float aThickness) noexcept;
		SectionItemBuilder& TextColor(const Color& aColor) noexcept;
	};

	class SeparatorItemBuilder : public ContextMenuItemBuilder<SeparatorItemBuilder>
	{
	public:
		explicit SeparatorItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept;

		SeparatorItemBuilder& Color(const DirectX::SimpleMath::Color& aColor) noexcept;

		SeparatorItemBuilder& Thickness(float aThickness) noexcept;
	};

	class SubmenuItemBuilder : public ContextMenuItemBuilder<SubmenuItemBuilder>
	{
	public:
		explicit SubmenuItemBuilder(ContextMenuBuilder& aParent, ContextMenuItem& aItem) noexcept;

		SubmenuItemBuilder& OnOpen(Callback<void(ContextMenuBuilder&)>&& aCallback) noexcept;
	};

	class ContextMenuBuilder
	{
	public:
		TextItemBuilder AddItem(StringView aName) noexcept;
		TextItemBuilder AddItem(StringView aName, Callback<void()>&& aCallback) noexcept;
		SectionItemBuilder AddSection(StringView aText) noexcept;
		SeparatorItemBuilder AddSeparator() noexcept;
		SubmenuItemBuilder AddSubmenu(StringView aName) noexcept;
		SubmenuItemBuilder AddSubmenu(StringView aName, Callback<void(ContextMenuBuilder&)>&& aCallback) noexcept;

		NO_DISCARD Ref<ContextMenu> BuildContextMenu() noexcept;
		NO_DISCARD Ref<ContextMenu> BuildSubMenu() noexcept;
	private:
		NO_DISCARD Ref<IBaseWidget> BuildSectionRow(const ContextMenuItem& aItem) noexcept;
		NO_DISCARD Ref<IBaseWidget> BuildSeparatorRow(const ContextMenuItem& aItem) noexcept;
		NO_DISCARD Ref<IBaseWidget> BuildSubMenuRow(ContextMenuItem& aItem) noexcept;
		NO_DISCARD Ref<IBaseWidget> BuildTextRow(const ContextMenuItem& aItem) noexcept;

		NO_DISCARD std::vector<Ref<IBaseWidget>> BuildRows() noexcept;
	private:
		std::vector<ContextMenuItem> m_Items;
	};
}