#pragma once
#include "IWidget.h"

namespace Relentless
{
	class Label : public IStylableWidget<Label>
	{
	public:
		Label(std::string_view text = "", ImFont* pFont = nullptr) noexcept;

		virtual NO_DISCARD float CalcDesiredWidth() const noexcept;
		NO_DISCARD const String& GetText() const noexcept;
		virtual void OnRender() noexcept override;
		Label* SetHighlightedSubstring(std::string_view text) noexcept;
		Label* SetText(std::string_view text) noexcept;
	private:
		void RenderHighlight() noexcept;
		void RenderText() noexcept;
	private:
		String m_Text;
		String m_HighlightedSubstring;
		ImFont* m_pFont = nullptr;
	};

}