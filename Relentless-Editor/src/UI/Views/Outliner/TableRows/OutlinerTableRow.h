#pragma once
#include "Subsystem/EntityFoldersSubsystem.h"

#include "UI/DragDrop/OutlinerDragDropOperation.h"
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

		NO_DISCARD bool IsDragDropEligible() noexcept override;

		template<typename InstanceType>
		OutlinerTableRow* OnDragDetected(InstanceType* instance, Ref<DragDropOperation>(InstanceType::*method)(OutlinerTableRow*)) noexcept
		{
			m_OnDragDetectedCallback = [instance, method](OutlinerTableRow* pRow) { return (instance->*method)(pRow); };
			return this;
		}

		template<typename InstanceType>
		OutlinerTableRow* OnDragEnter(InstanceType* instance, bool(InstanceType::*method)(OutlinerTableRow*, OutlinerDragDropOperation&)) noexcept
		{
			m_OnDragEnterCallback = [instance, method](OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) { return (instance->*method)(pRow, dragDropOp); };
			return this;
		}

		template<typename InstanceType>
		OutlinerTableRow* OnDragLeave(InstanceType* instance, void(InstanceType::*method)(OutlinerTableRow*, OutlinerDragDropOperation&)) noexcept
		{
			m_OnDragLeaveCallback = [instance, method](OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) { return (instance->*method)(pRow, dragDropOp); };
			return this;
		}

		template<typename InstanceType>
		OutlinerTableRow* OnDrop(InstanceType* instance, bool(InstanceType::*method)(OutlinerTableRow*, OutlinerDragDropOperation&)) noexcept
		{
			m_OnDropCallback = [instance, method](OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) { return (instance->*method)(pRow, dragDropOp); };
			return this;
		}

	private:
		NO_DISCARD Ref<DragDropOperation> OnDragDetected() noexcept override;
		NO_DISCARD bool OnDragEnter(const Ref<DragDropOperation>& pDragDropOperation) noexcept override;
		NO_DISCARD bool OnDragLeave(const Ref<DragDropOperation>& pDragDropOperation) noexcept override;
		NO_DISCARD bool OnDrop(const Ref<DragDropOperation>& pDragDropOperation) noexcept override;

	private:
		std::array<FloatRect, 3> m_Margins;

		Callback<Ref<DragDropOperation>(OutlinerTableRow*)> m_OnDragDetectedCallback;
		Callback<bool(OutlinerTableRow*, OutlinerDragDropOperation&)> m_OnDragEnterCallback;
		Callback<void(OutlinerTableRow*, OutlinerDragDropOperation&)> m_OnDragLeaveCallback;
		Callback<bool(OutlinerTableRow*, OutlinerDragDropOperation&)> m_OnDropCallback;

		TreeView<Ref<OutlinerListItem>>* m_pOwningTreeView = nullptr;
		bool m_Selected = false;
	};
}