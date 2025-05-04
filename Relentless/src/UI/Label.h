#pragma once
#include "IWidget.h"

namespace Relentless
{
	class Label : public IWidget
	{
	public:
		Label(std::string_view id) noexcept;

		virtual void OnRender() noexcept override;

		void SetText(std::string_view text) noexcept;
	private:
		String m_Text;
	};

}