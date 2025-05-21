#pragma once
#include "IWidget.h"

#include "Callback/Broadcaster.h"
#include "Callback/Callback.h"

namespace Relentless
{
	class Texture;

	class Canvas : public IStylableWidget
	{
	public:
		Canvas(std::string_view id) noexcept;
		virtual ~Canvas() noexcept override = default;

		[[nodiscard]] IntRect GetScreenRect() const noexcept;

		template<typename InstanceType>
		Canvas* OnResize(InstanceType* instance, void(InstanceType::* method)(const Vector2i&)) noexcept
		{
			m_OnResizeCallback = [instance, method](const Vector2i& newSize) { return (instance->*method)(newSize); };
			return this;
		}

		template<typename InstanceType>
		Canvas* Target(InstanceType* instance, Texture*(InstanceType::* method)() const) noexcept
		{
			m_TargetCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		template<typename InstanceType>
		Canvas* OnHoverStateChanged(InstanceType* instance, void(InstanceType::* method)(bool)) noexcept
		{
			m_OnHoverStateChanged = [instance, method](bool state) { return (instance->*method)(state); };
			return this;
		}

	protected:
		[[nodiscard]] float CalcDesiredWidth() const noexcept override;
	private:
		virtual void OnRender() noexcept override;
		void Resize(const Vector2i& newSize) noexcept;
	private:
		Callback<void(bool state)> m_OnHoverStateChanged;
		Callback<void(const Vector2i& newSize)> m_OnResizeCallback;
		Callback<Texture*()> m_TargetCallback;

		IntRect m_ScreenRect;
		Vector2i m_Size = Vector2i::Zero();
		bool m_IsHovered = false;
	};
}