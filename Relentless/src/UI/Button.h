#pragma once
#include "IWidget.h"
#include "Callback/Broadcaster.h"

namespace Relentless
{
	class Button : public IWidget
	{
	public:
		Button(std::string_view id, const Vector2& size = Vector2::Zero) noexcept;

		virtual void OnRender() noexcept override;
		void SetActiveColor(const Color& color) noexcept;
		void SetColor(const Color& color) noexcept;
		void SetHoverColor(const Color& color) noexcept;
		void SetSize(const Vector2& size) noexcept;

		Broadcaster<void()> OnClicked;
	private:
		void SetColorsAndStyles() noexcept;
	private:
		Color m_Color = Colors::Gray;
		Color m_HoverColor = Colors::Gray;
		Color m_ActiveColor = Colors::Gray;

		Vector2 m_Size = Vector2::Zero;
	};
}