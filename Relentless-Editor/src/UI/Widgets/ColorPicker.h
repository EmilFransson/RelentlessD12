#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class ColorPicker : public IStylableWidget<ColorPicker>
	{
	public:
		ColorPicker(const Vector2& size = Vector2(0.0f, 0.0f), int flags = 0) noexcept;

		NO_DISCARD virtual float CalcDesiredWidth() const noexcept override;

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

		void SetColorPickerFlags(int flags) noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		Callback<Color()> m_ValueCallback;
		Callback<void(Color color)> m_OnChanged;

		Vector2 m_Size = Vector2::Zero;
		int m_PickerFlags = 0;
		bool m_IsHovered = false;
	};
}