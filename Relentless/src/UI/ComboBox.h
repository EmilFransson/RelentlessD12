#pragma once
#include "IWidget.h"

namespace Relentless
{
	class ComboBox : public IStylableWidget
	{
	public:
		ComboBox(std::string_view id, int flags = 0) noexcept;

		void AddSelectables(Span<const char*> selectables) noexcept;
		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] int GetSelectedIndex() const;

		void SetDropDownButtonColor(const Color& color) noexcept;
		void SetDropDownButtonActiveColor(const Color& color) noexcept;
		void SetDropDownButtonHoveredColor(const Color& color) noexcept;

		void SetSelectableBackgroundColor(const Color& color) noexcept;

		Broadcaster<void(int selected)> OnChanged;
	protected:
		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;
	private:
		std::vector<const char*> m_Selectables;
		int m_Selected = 0;
		bool m_IsHovered = false;
	};
}