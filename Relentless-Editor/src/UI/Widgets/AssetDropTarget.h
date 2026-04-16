#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class AssetDropTarget : public IWidget<AssetDropTarget>
	{
	public:
		AssetDropTarget() noexcept;
		virtual ~AssetDropTarget() noexcept override;

		template<typename T>
		AssetDropTarget* OnAreAssetsAcceptable(T&& aCallback) noexcept;

		template<typename T>
		AssetDropTarget* OnAssetsDropped(T&& aCallback) noexcept;
		
		NO_DISCARD virtual Vector2 ReportSize() const noexcept override;
		NO_DISCARD bool RequiresAssignedSize() const noexcept override;

		template<typename WidgetType>
		WidgetType* Slot(Ref<WidgetType> aWidget) noexcept;

		template<typename WidgetType>
		WidgetType* Slot(WidgetType* aWidget) noexcept;
	protected:
		void OnRender() noexcept override;
	private:
		void OnSlotDragEnter(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		void OnSlotDragLeave(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnSlotDragOver(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnSlotDrop(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;

		void OnDragDropOperationBegin(const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		void OnDragDropOperationEnd(MAYBE_UNUSED  const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
	private:
		Callback<bool(Span<const AssetData>)> m_OnAreAssetsAcceptableCallback;
		Callback<void(Span<const AssetData>)> m_OnAssetsDroppedCallback;

		Color m_DropAreaOverlayColor = Colors::Green;
		bool m_DrawDropAreaOverlay = false;
		Ref<IBaseWidget> m_pWidget = nullptr;
	};

	template<typename T>
	AssetDropTarget* AssetDropTarget::OnAreAssetsAcceptable(T&& aCallback) noexcept
	{
		m_OnAreAssetsAcceptableCallback = Callback<bool(Span<const AssetData>)>(std::forward<T>(aCallback));
		return this;
	}

	template<typename T>
	AssetDropTarget* AssetDropTarget::OnAssetsDropped(T&& aCallback) noexcept
	{
		m_OnAssetsDroppedCallback = Callback<void(Span<const AssetData>)>(std::forward<T>(aCallback));
		return this;
	}

	template<typename WidgetType>
	WidgetType* AssetDropTarget::Slot(Ref<WidgetType> aWidget) noexcept
	{
		RLS_ASSERT(aWidget, "[AssetDropTarget::Slot]: Widget is invalid.");
		static_assert(std::is_base_of_v<IBaseWidget, WidgetType>, "[AssetDropTarget::Slot]: sWidgetType must inherit from IBaseWidget.");

		m_pWidget = aWidget;
		m_pWidget->OnDragEnter(this, &AssetDropTarget::OnSlotDragEnter);
		m_pWidget->OnDragLeave(this, &AssetDropTarget::OnSlotDragLeave);
		m_pWidget->OnDragOver(this, &AssetDropTarget::OnSlotDragOver);
		m_pWidget->OnDrop(this, &AssetDropTarget::OnSlotDrop);

		return static_cast<WidgetType*>(m_pWidget.Get());
	}

	template<typename WidgetType>
	WidgetType* AssetDropTarget::Slot(WidgetType* aWidget) noexcept
	{
		RLS_ASSERT(aWidget, "[AssetDropTarget::Slot]: Widget is invalid.");
		static_assert(std::is_base_of_v<IBaseWidget, WidgetType>, "[AssetDropTarget::Slot]: sWidgetType must inherit from IBaseWidget.");
		return Slot(Ref<WidgetType>(aWidget));
	}
}