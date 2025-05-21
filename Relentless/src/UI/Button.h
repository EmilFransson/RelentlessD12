#pragma once
#include "IWidget.h"
#include "Callback/Callback.h"

namespace Relentless
{
	class Button : public IStylableWidget
	{
	public:
		Button(std::string_view id, const Vector2& size = Vector2::Zero) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		virtual void SetActiveColor(const Color& color) noexcept override;
		virtual void SetBackgroundColor(const Color& color) noexcept override;
		virtual void SetHoverColor(const Color& color) noexcept override;
		void SetSize(const Vector2& size) noexcept;

		template<typename InstanceType>
		Button* OnClicked(InstanceType* instance, void(InstanceType::* method)()) noexcept
		{
			m_OnClickedCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		template<typename T>
		Button* OnClicked(T&& callback) noexcept
		{
			m_OnClickedCallback = Callback<void()>(std::forward<T>(callback));
			return this;
		}
	protected:
		virtual void OnRender() noexcept override;
	private:
		Callback<void()> m_OnClickedCallback;
		Vector2 m_Size = Vector2::Zero;
	};
}