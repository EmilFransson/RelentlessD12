#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class Label : public IStylableWidget<Label>
	{
	public:
		Label(std::string_view text = "", ImFont* pFont = nullptr) noexcept;

		NO_DISCARD const String& GetText() const noexcept;

		virtual void OnRender() noexcept override;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		Label* SetHighlightedSubstring(std::string_view text) noexcept;
		Label* SetText(std::string_view text) noexcept;
	private:
		void RenderHighlight() noexcept;
		void RenderText() noexcept;
	private:
		String m_Text;
		String m_HighlightedSubstring;
	};

}