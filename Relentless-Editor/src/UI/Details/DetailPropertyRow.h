#pragma once
#include "UI/Widgets/IWidget.h"

namespace Relentless
{
	class Label;

	class DetailPropertyRow : public ICompoundWidget<DetailPropertyRow>
	{
	public:
		DetailPropertyRow() noexcept;
		virtual ~DetailPropertyRow() noexcept override = default;

		NO_DISCARD virtual float CalcDesiredWidth() const noexcept override { return 0.0f; };

		virtual void OnRender() noexcept override;

		template<typename WidgetType>
		WidgetType* SetNameContent(WidgetType* pWidget) noexcept
		{
			static_assert(std::is_base_of<IBaseWidget, WidgetType>::value, "[DetailPropertyRow::SetNameContent]: WidgetType is not derived from IWidget.");

			m_pNameContentSlot = Ref<WidgetType>(pWidget);
			return pWidget;
		}

		template<typename WidgetType>
		WidgetType* SetValueContent(WidgetType* pWidget) noexcept
		{
			static_assert(std::is_base_of<IBaseWidget, WidgetType>::value, "[DetailPropertyRow::SetValueContent]: WidgetType is not derived from IWidget.");

			m_pValueContentSlot = Ref<WidgetType>(pWidget);
			return pWidget;
		}

	private:
		Ref<IBaseWidget> m_pNameContentSlot = nullptr;
		Ref<IBaseWidget> m_pValueContentSlot = nullptr;
		Ref<Label> m_pResetToDefaultSlot = nullptr;
	};
}