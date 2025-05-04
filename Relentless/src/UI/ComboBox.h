#pragma once
#include "IWidget.h"

namespace Relentless
{
	class ComboBox : public IWidget
	{
	public:
		ComboBox(std::string_view id, int flags = 0) noexcept;

		void AddSelectables(Span<const char*> selectables) noexcept;
		[[nodiscard]] int GetSelectedIndex() const;

		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;

		Broadcaster<void(int selected)> OnChanged;
	private:
		void SetColorsAndStyles() noexcept;
	private:
		std::vector<const char*> m_Selectables;
		int m_Selected = 0;
		bool m_IsHovered = false;
	};
}