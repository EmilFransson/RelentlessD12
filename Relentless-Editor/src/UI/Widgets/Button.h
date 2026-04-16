#pragma once
#include <Relentless.h>
#include "IStylableWidget.h"

namespace Relentless
{
	class Button : public IStylableWidget<Button>
	{
	public:
		Button(std::string_view aText, const Vector2& aSize = Vector2::Zero) noexcept;

		NO_DISCARD const String& GetText() const noexcept;

		NO_DISCARD virtual Vector2 ReportSize() const noexcept override;

		virtual Button* SetActiveColor(const Color& aColor) noexcept override;
		virtual Button* SetBackgroundColor(const Color& aColor) noexcept override;
		virtual Button* SetHoverColor(const Color& aColor) noexcept override;
		void SetText(const String& aText) noexcept;

		template<typename InstanceType>
		Button* OnClicked(InstanceType* aInstance, void(InstanceType::*aMethod)()) noexcept
		{
			m_OnClickedCallback = [aInstance, aMethod]() { return (aInstance->*aMethod)(); };
			return this;
		}

		template<typename T>
		Button* OnClicked(T&& aCallback) noexcept
		{
			m_OnClickedCallback = Callback<void()>(std::forward<T>(aCallback));
			return this;
		}

	protected:
		virtual void OnRender() noexcept override;

		NO_DISCARD bool RequiresAssignedSize() const noexcept override;
	private:
		String m_Text;
		Callback<void()> m_OnClickedCallback;
		Vector2 m_TextSize = Vector2::Zero;
		Vector2 m_Size = Vector2::Zero;
	};
}