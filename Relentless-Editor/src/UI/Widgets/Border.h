#pragma once
#include "IWidget.h"

namespace Relentless
{
	class Border : public IStylableWidget<Border>
	{
	public:
		Border() noexcept = default;
		
		virtual float CalcDesiredWidth() const noexcept override;

		NO_DISCARD Ref<IBaseWidget> GetContent() const noexcept;

		virtual void OnRender() noexcept override;

		NO_DISCARD virtual Vector2 ReportSize() const noexcept override;

		template<typename T>
		T* SetContent(T* pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[Border::SetContent]: Can only Add widgets derived from IBaseWidget");

			m_pContent = pWidget;
			return static_cast<T*>(m_pContent.Get());
		}

		template<typename T>
		T* SetContent(Ref<T> pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[Border::SetContent]: Can only Add widgets derived from IBaseWidget");

			m_pContent = pWidget;
			return static_cast<T*>(m_pContent.Get());
		}
	private:
		Ref<IBaseWidget> m_pContent = nullptr;
	};
}