#pragma once
#include "IWidget.h"

namespace Relentless
{
	class Label : public IStylableWidget
	{
	public:
		Label(std::string_view id, ImFont* pFont = nullptr) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept;
		virtual void OnRender() noexcept override;

		void SetText(std::string_view text) noexcept;
	private:
		String m_Text;
		ImFont* m_pFont = nullptr;
	};

}