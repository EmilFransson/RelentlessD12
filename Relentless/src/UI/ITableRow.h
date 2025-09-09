#pragma once
#include "HorizontalBox.h"

#include "Callback/Callback.h"
#include "Core/Any.h"
#include "Input/Mouse.h"

namespace Relentless
{
	class DragDropOperation;

	class ITableRow : public IStylableWidget<ITableRow>
	{
	public:
		ITableRow() noexcept = default;
		virtual ~ITableRow() noexcept = default;

		template<typename T>
		ITableRow* OnClicked(T&& callback) noexcept
		{
			m_OnClickedCallback = Callback<void(const PointerInfo&)>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		ITableRow* OnClicked(InstanceType* instance, void(InstanceType::* method)(const PointerInfo&)) noexcept
		{
			m_OnClickedCallback = [instance, method](const PointerInfo& pointerInfo) { return (instance->*method)(pointerInfo); };
			return this;
		}

		template<typename T>
		ITableRow* OnDoubleClicked(T&& callback) noexcept
		{
			m_OnDoubleClickedCallback = Callback<void()>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		ITableRow* OnDoubleClicked(InstanceType* instance, void(InstanceType::* method)()) noexcept
		{
			m_OnDoubleClickedCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		virtual const Color& GetBackgroundColor() const noexcept = 0;

		virtual uint32 GetNumColumns() noexcept = 0;
		virtual bool IsDragDropEligible() noexcept = 0;

		virtual NO_DISCARD Ref<DragDropOperation> OnDragDetected() noexcept { return nullptr; }
		virtual NO_DISCARD bool OnDragEnter(const Ref<DragDropOperation>& pDragDropOperation) noexcept { return false; }
		virtual NO_DISCARD bool OnDragLeave(const Ref<DragDropOperation>& pDragDropOperation) noexcept { return false; }
		virtual NO_DISCARD bool OnDrop(const Ref<DragDropOperation>& pDragDropOperation) noexcept { return false; }

		virtual void OnRender() noexcept override final;
		virtual void OnRenderColumn(uint32 column) noexcept = 0;

	protected:
		std::vector<Ref<HorizontalBox>> m_ColumnWidgets;

		Callback<void(const PointerInfo& pointerInfo)> m_OnClickedCallback;
		Callback<void()> m_OnDoubleClickedCallback;
		
		bool m_Hovered = false;
	};
}