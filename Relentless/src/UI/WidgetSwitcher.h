#pragma once

#include "IWidget.h"

namespace Relentless
{
	class WidgetSwitcher : public IWidget<WidgetSwitcher>
	{
	public:
		template<typename T>
		T* Add(T* pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[WidgetSwitcher::Add]: Can only Add widgets derived from IBaseWidget");

			Ref<T> widgetRef(pWidget);
			m_Widgets.push_back(widgetRef);
			return widgetRef.Get();
		}

		template<typename T>
		T* Add(Ref<T> pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[WidgetSwitcher::Add]: Can only Add widgets derived from IBaseWidget");

			m_Widgets.push_back(pWidget);
			return pWidget.Get();
		}

		virtual NO_DISCARD float CalcDesiredWidth() const noexcept override;

		NO_DISCARD Ref<IBaseWidget> GetActiveWidget() const noexcept;
		NO_DISCARD int32 GetActiveWidgetIndex() noexcept;
		NO_DISCARD uint32 GetNumWidgets() const noexcept;
		NO_DISCARD Ref<IBaseWidget> GetWidget(uint32 aSlotIndex) const noexcept;
		NO_DISCARD int32 GetWidgetIndex(Ref<IBaseWidget> aWidget) const noexcept;

		virtual void OnRender() noexcept override;
		void SetActiveWidget(Ref<IBaseWidget> aWidget) noexcept;
		void SetActiveWidgetIndex(uint32 aSlotIndex) noexcept;
	private:
		std::vector<Ref<IBaseWidget>> m_Widgets;
		int32 m_ActiveIndex = -1;
	};
}