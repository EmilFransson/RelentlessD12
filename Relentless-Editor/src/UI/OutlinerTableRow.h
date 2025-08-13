#pragma once

#include <Relentless.h>

#include "../Core/EntityFilters.h"

namespace Relentless
{
	struct OutlinerListItem : public RefCounted<OutlinerListItem>
	{
		entity Entity = NULL_ENTITY;
		std::shared_ptr<EntityFilter> pFilter = nullptr;

		NO_DISCARD static const String& GetEntityTypeAsString() { static String str = "Entity";  return str; }
		NO_DISCARD static const String& GetFilterTypeAsString() { static String str = "Filter"; return str; }

		NO_DISCARD bool IsEntityItem() const noexcept { return Entity != NULL_ENTITY; }
		NO_DISCARD bool IsFilterItem() const noexcept { return pFilter != nullptr; }
	};

	class OutlinerTableRow : public ITableRow
	{
	public:
		OutlinerTableRow(ListView<Ref<OutlinerListItem>>* pListView) noexcept;
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

		NO_DISCARD Ref<DragDropOperation> OnDragDetected() noexcept override;
		NO_DISCARD bool OnDragEnter(const Ref<DragDropOperation>& pDragDropOperation) noexcept override;
		NO_DISCARD bool OnDragLeave(const Ref<DragDropOperation>& pDragDropOperation) noexcept override;
		NO_DISCARD bool OnDrop(const Ref<DragDropOperation>& pDragDropOperation) noexcept override;

	private:
		std::array<Ref<IBaseWidget>, 3> m_ColumnWidgets;
		std::array<FloatRect, 3> m_Margins;

		ListView<Ref<OutlinerListItem>>* m_pOwningListView = nullptr;
		Ref<Tooltip> m_pDragDropTooltip = nullptr;
		bool m_Selected = false;
	};
}