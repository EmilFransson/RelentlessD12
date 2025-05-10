#pragma once
#include "IWidget.h"

namespace Relentless
{
	class ColorPicker : public IStylableWidget
	{
	public:
		ColorPicker(std::string_view id, const Color& initialColor, const Vector2& size = Vector2(0.0f, 0.0f), int flags = 0) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		virtual void OnRender() noexcept override;

		void SetColorPickerFlags(int flags) noexcept;

		Broadcaster<void(const Color& color)> OnChanged;
	private:
		Color m_Color = Colors::White;
		Vector2 m_Size = Vector2::Zero;
		int m_PickerFlags = 0;
		bool m_IsHovered = false;
	};
}