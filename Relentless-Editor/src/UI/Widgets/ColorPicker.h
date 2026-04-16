#pragma once
#include <Relentless.h>
#include "IStylableWidget.h"

namespace Relentless
{
	template<typename DataType> class PropertyHandle;

	class ColorPicker : public IStylableWidget<ColorPicker>
	{
	public:
		ColorPicker(int flags = 0) noexcept;

		ColorPicker* Bind(Ref<PropertyHandle<Color>> aPropertyHandle) noexcept;

		template<typename T>
		ColorPicker* OnValueChanged(T&& callBack) noexcept 
		{
			m_OnChanged = Callback<void(Color)>(std::forward<T>(callBack));
			return this;
		}

		template<typename InstanceType>
		ColorPicker* OnValueChanged(InstanceType* instance, void(InstanceType::* method)(const Color&)) noexcept
		{
			m_OnChanged = [instance, method](const Color& color) { return (instance->*method)(color); };
			return this;
		}

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		template<typename T>
		ColorPicker* Value(T&& callBack) noexcept 
		{
			m_ValueCallback = Callback<Color()>(std::forward<T>(callBack));
			return this;
		}

		template<typename InstanceType>
		ColorPicker* Value(InstanceType* instance, Color(InstanceType::* method)() const) noexcept
		{
			m_ValueCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		bool RequiresAssignedSize() const noexcept override;
		
		void SetColorPickerFlags(int flags) noexcept;

	protected:
		virtual void OnRender() noexcept override;
	private:
		NO_DISCARD Vector2 GetRenderedSize() const noexcept;
	private:
		Callback<Color()> m_ValueCallback;
		Callback<void(Color color)> m_OnChanged;

		int m_PickerFlags = 0;
		bool m_IsHovered = false;

		Ref<PropertyHandle<Color>> m_pPropertyHandle = nullptr;
	};
}