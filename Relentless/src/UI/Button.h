#pragma once
#include "IWidget.h"
#include "Callback/Broadcaster.h"

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

		Broadcaster<void()> OnClicked;
	protected:
		virtual void OnRender() noexcept override;
	private:
		Vector2 m_Size = Vector2::Zero;
	};
}