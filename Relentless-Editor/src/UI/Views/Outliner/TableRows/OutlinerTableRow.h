#pragma once
#include "Subsystem/EntityFoldersSubsystem.h"

#include "UI/Views/TreeView.h"
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	using OutlinerPayload = std::variant<entity, EntityFolder*, Scene*>;

	struct OutlinerListItem : public RefCounted<OutlinerListItem>
	{
		std::vector<Ref<OutlinerListItem>> Children;
		OutlinerPayload Payload;

		NO_DISCARD static constexpr std::string_view GetEntityTypeAsString() { return "Entity"; }
		NO_DISCARD static constexpr std::string_view GetFolderTypeAsString() { return "Folder"; }
		NO_DISCARD static constexpr std::string_view GetSceneTypeAsString() { return "Scene"; }

		NO_DISCARD bool IsEntity() const noexcept { return std::holds_alternative<entity>(Payload); }
		NO_DISCARD bool IsFolder() const noexcept { return std::holds_alternative<EntityFolder*>(Payload); }
		NO_DISCARD bool IsScene() const noexcept { return std::holds_alternative<Scene*>(Payload); }

		NO_DISCARD entity AsEntity() const noexcept { RLS_ASSERT(IsEntity(), "[OutlinerListItem::AsEntity]: Not an entity item!"); return std::get<entity>(Payload); }
		NO_DISCARD EntityFolder* AsFolder() const noexcept { RLS_ASSERT(IsFolder(), "[OutlinerListItem::AsFolder]: Not a folder item!"); return std::get<EntityFolder*>(Payload); }
		NO_DISCARD Scene* AsScene() const noexcept { RLS_ASSERT(IsScene(), "[OutlinerListItem::AsScene]: Not a scene item!"); return std::get<Scene*>(Payload); }
	};

	struct OutlinerTableRowCreateInfo
	{
		String Icon;
		String Name;
		String Type;
		Color IconColor = Colors::White;
		bool IsVisible = false;
		bool IsSelected = false;
		bool HasChildren = false;
		bool IsExpanded = false;
		TreeView<Ref<OutlinerListItem>>* pTreeView = nullptr;
	};

	class Button;
	class EditableTextBox;
	class Label;
	class WidgetSwitcher;

	class OutlinerTableRow : public ITableRow
	{
	public:
		OutlinerTableRow(const OutlinerTableRowCreateInfo& createInfo) noexcept;
		virtual ~OutlinerTableRow() noexcept override = default;

		NO_DISCARD Ref<IBaseWidget> GetWidget(uint8 column) noexcept;
		
		virtual void OnRenderColumn(uint32 column) noexcept override;

		NO_DISCARD const Color& GetBackgroundColor() const noexcept override;
		NO_DISCARD EditableTextBox* GetEditableTextBox() const noexcept;
		NO_DISCARD Button* GetExpandButton() const noexcept;
		NO_DISCARD Label* GetNameLabel() const noexcept;
		NO_DISCARD uint32 GetNumColumns() noexcept override;
		NO_DISCARD Label* GetTypeLabel() const noexcept;
		NO_DISCARD WidgetSwitcher* GetWidgetSwitcher() const noexcept;
		NO_DISCARD Button* GetVisibilityButton() const noexcept;
	private:
		std::array<FloatRect, 3> m_Margins;
		TreeView<Ref<OutlinerListItem>>* m_pOwningTreeView = nullptr;
		bool m_Selected = false;
	};
}