#pragma once

#include <Relentless.h>

#include "../Core/EntityFilters.h"
#include "DragDrop/OutlinerDragDropOperation.h"

namespace Relentless
{
	struct OutlinerListItem : public RefCounted<OutlinerListItem>
	{
		std::vector<Ref<OutlinerListItem>> Children;

		entity Entity = NULL_ENTITY;
		EntityFilter* pFilter = nullptr;
		Scene* pScene = nullptr;

		NO_DISCARD static const String& GetEntityTypeAsString() { static String str = "Entity";  return str; }
		NO_DISCARD static const String& GetFilterTypeAsString() { static String str = "Filter"; return str; }
		NO_DISCARD static const String& GetSceneTypeAsString() { static String str = "Scene"; return str; }

		NO_DISCARD bool IsEntityItem() const noexcept { return Entity != NULL_ENTITY; }
		NO_DISCARD bool IsFilterItem() const noexcept { return pFilter != nullptr; }
		NO_DISCARD bool IsSceneItem() const noexcept { return pScene != nullptr; }
	};

	class OutlinerTableRow : public ITableRow
	{
	public:
		OutlinerTableRow(TreeView<Ref<OutlinerListItem>>* pTreeView) noexcept;
		virtual ~OutlinerTableRow() noexcept override = default;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;
		NO_DISCARD Ref<IBaseWidget> GetWidget(uint8 column) noexcept;
		
		virtual void OnRenderColumn(uint32 column) noexcept override;

		template<typename WidgetType>
		WidgetType* SetWidget(WidgetType* pWidget, uint8 column) noexcept
		{
			static_assert(std::is_base_of<IBaseWidget, WidgetType>::value, "[OutlinerTableRow::SetWidget]: WidgetType is not derived from IWidget.");

			m_ColumnWidgets[column] = Ref<WidgetType>(pWidget);
			return pWidget;
		}

		NO_DISCARD const Color& GetBackgroundColor() const noexcept override;
		NO_DISCARD uint32 GetNumColumns() noexcept override;
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
		std::array<Ref<IBaseWidget>, 3> m_ColumnWidgets;
		std::array<FloatRect, 3> m_Margins;

		Callback<Ref<DragDropOperation>(OutlinerTableRow*)> m_OnDragDetectedCallback;
		Callback<bool(OutlinerTableRow*, OutlinerDragDropOperation&)> m_OnDragEnterCallback;
		Callback<void(OutlinerTableRow*, OutlinerDragDropOperation&)> m_OnDragLeaveCallback;
		Callback<bool(OutlinerTableRow*, OutlinerDragDropOperation&)> m_OnDropCallback;

		TreeView<Ref<OutlinerListItem>>* m_pOwningTreeView = nullptr;
		bool m_Selected = false;
	};
}