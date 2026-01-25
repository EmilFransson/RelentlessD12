#pragma once

namespace Relentless
{
	class Tooltip;

	class DragDropOperation : public RefCounted<DragDropOperation>
	{
	public:
		virtual ~DragDropOperation() noexcept = default;

		template<class T>
		T& As() noexcept
		{
			RLS_ASSERT(Is<T>(), "[DragDropOperation::As]: Object is not of type T.");
			return static_cast<T&>(*this);
		}

		template<class T>
		const T& As() const noexcept
		{
			RLS_ASSERT(Is<T>(), "[DragDropOperation::As]: Object is not of type T.");
			return static_cast<const T&>(*this);
		}

		NO_DISCARD virtual uint64 GetType() const noexcept = 0;           // hashed type key
		NO_DISCARD virtual const char* GetTypeName() const noexcept = 0;  // for logs/tooltips

		template<class T> bool Is() const noexcept 
		{
			static_assert(requires { T::kType; }, "T must define static constexpr uint64 kType");
			return GetType() == T::kType;
		}

		void SetTooltipText(std::string_view text) noexcept;

		void Update() noexcept;

	private:
		Ref<Tooltip> m_pTooltip = nullptr;
	};
}