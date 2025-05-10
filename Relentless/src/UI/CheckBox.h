#pragma once
#include "IWidget.h"
#include "Callback/Broadcaster.h"

namespace Relentless
{
	class CheckBox : public IStylableWidget
	{
	public:
		CheckBox(std::string_view id) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		Broadcaster<void(bool isChecked)> OnCheckStateChanged;
	protected:
		virtual void OnRender() noexcept override;
	private:
		bool m_State = false;
		bool m_Hovered = false;
	};
}