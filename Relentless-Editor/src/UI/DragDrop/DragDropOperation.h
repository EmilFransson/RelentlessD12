#pragma once
#include <Relentless.h>

namespace Relentless
{
	class IBaseWidget;
	class Tooltip;

	class DragDropOperationBase : public RefCounted<DragDropOperationBase>
	{
	public:
		DragDropOperationBase() noexcept = default;
		virtual ~DragDropOperationBase() noexcept;

		template<typename T>
		T& AsType() noexcept
		{
			RLS_ASSERT(IsOfType<T>(), "[DragDropOperation::As]: Object is not of type T.");
			return static_cast<T&>(*this);
		}

		virtual void CreatePreview() noexcept;

		NO_DISCARD const Ref<IBaseWidget> GetPreview() const noexcept;
		virtual const TypeIndex& GetStaticType() const noexcept = 0;

		template<typename T>
		NO_DISCARD bool IsOfType() noexcept
		{
			return T::StaticType() == GetStaticType();
		}
	protected:
		Ref<IBaseWidget> m_pPreviewWidget;
	};

	template<typename DragDropType>
	class DragDropOperation : public DragDropOperationBase
	{
	public:
		virtual ~DragDropOperation() noexcept override = default;

		virtual const TypeIndex& GetStaticType() const noexcept override final
		{
			return StaticType();
		}

		static constexpr const TypeIndex& StaticType()
		{
			static constexpr TypeIndex typeIndex = getTypeIndex<DragDropType>();
			return typeIndex;
		}
	};

	class Reply
	{
	public:
		NO_DISCARD static Reply Handled() noexcept
		{
			Reply reply;
			reply.m_Handled = true;
			return reply;
		}

		NO_DISCARD Reply BeginDragDrop(Ref<DragDropOperationBase> aDragDropOperation) noexcept
		{
			m_pDragDropOperation = aDragDropOperation;
			return *this;
		}

		NO_DISCARD Ref<DragDropOperationBase> GetDragDropOperation() const noexcept
		{
			return m_pDragDropOperation;
		}

		NO_DISCARD bool IsHandled() const noexcept { return m_Handled; }

		static Reply Unhandled() noexcept
		{
			Reply reply;
			reply.m_Handled = false;
			return reply;
		}
	private:
		Reply() noexcept = default;
	private:
		bool m_Handled = false;
		Ref<DragDropOperationBase> m_pDragDropOperation = nullptr;
	};
}