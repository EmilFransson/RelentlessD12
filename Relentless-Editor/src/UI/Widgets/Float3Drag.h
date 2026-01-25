#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class HorizontalBox;

	class Float3Drag : public IStylableWidget<Float3Drag>
	{
	public:
		Float3Drag(float speed = 1.0f, float min = -FLT_MAX, float max = FLT_MAX, const char* pFormat = "", int flags = 0) noexcept;

		NO_DISCARD virtual float CalcDesiredWidth() const noexcept override;

		template<typename InstanceType>
		Float3Drag* Value(InstanceType* instance, Vector3(InstanceType::*method)()) noexcept;

		template<typename T>
		Float3Drag* Value(T&& callback) noexcept;

		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;

		template<typename T>
		Float3Drag* OnValueChanged(T&& callback) noexcept;

		template<typename InstanceType>
		Float3Drag* OnValueChanged(InstanceType* instance, void(InstanceType::*method)(Vector3&)) noexcept;

		Float3Drag* SetDrawColorIndicator(bool state) noexcept;
		Float3Drag* SetHandleColor(const Color& color) noexcept;
		Float3Drag* SetHandleSize(float size) noexcept;
		Float3Drag* SetIndicatorColor(uint8 handleIndex, const Color& color) noexcept;

		Broadcaster<void(bool state)> OnActiveChanged;


	private:
		[[nodiscard]] float GetValue(int componentIndex) const noexcept;


		[[nodiscard]] bool IsAnyHovered() const noexcept;
		[[nodiscard]] bool IsAnyUsed() const noexcept;

		void SetActive(bool state, uint8 componentIndex) noexcept;
		void SetValue(float value, int componentIndex) noexcept;

		void RenderComponent(float& value, uint8 componentIndex) noexcept;
		void OnMouseRawMove(const Vector2i& delta) noexcept;
	private:
		String m_Format;

		Callback<Vector3()> m_ValueCallback;
		Callback<void(Vector3& value)> m_OnChanged;

		std::array<Color, 3> m_IndicatorColors = { Colors::White };
		std::array<bool, 3> m_IsActiveAxis = { false };
		std::array<bool, 3> m_IsHoveredAxis = { false };
		std::array<bool, 3> m_IsUsingAxis = { false };

		Vector2i m_Delta = Vector2i::Zero();

		float m_Min = 0.0f;
		float m_Max = 0.0f;
		float m_Speed = 1.0f;

		bool m_DrawColorIndicator = false;

		Ref<HorizontalBox> m_pFloatDragBox = nullptr;
	};

	template<typename T>
	Float3Drag* Float3Drag::OnValueChanged(T&& callback) noexcept
	{
		m_OnChanged = Callback<void(Vector3&)>(std::forward<T>(callback));
		return this;
	}

	template<typename InstanceType>
	Float3Drag* Float3Drag::OnValueChanged(InstanceType* instance, void(InstanceType::*method)(Vector3&)) noexcept
	{
		m_OnChanged = [instance, method](Vector3& aValue) { return (instance->*method)(aValue); };
		return this;
	}

	template<typename T>
	Float3Drag* Float3Drag::Value(T&& callback) noexcept
	{
		m_ValueCallback = Callback<Vector3()>(std::forward<T>(callback));
		return this;
	}

	template<typename InstanceType>
	Float3Drag* Float3Drag::Value(InstanceType* instance, Vector3(InstanceType::*method)()) noexcept
	{
		m_ValueCallback = [instance, method]() { return (instance->*method)(); };
		return this;
	}

}