#pragma once
#include "IWidget.h"
#include "Callback/Broadcaster.h"

namespace Relentless
{
	class CheckBox : public IWidget
	{
	public:
		CheckBox(std::string_view id) noexcept;

		virtual void OnRender() noexcept override;

		Broadcaster<void(bool isChecked)> OnCheckStateChanged;
	private:
		void SetColorsAndStyles() noexcept;
	private:
		bool m_State = false;
		bool m_Hovered = false;
	};
}