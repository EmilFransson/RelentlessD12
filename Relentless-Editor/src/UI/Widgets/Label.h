#pragma once
#include <Relentless.h>
#include "IStylableWidget.h"

namespace Relentless
{
	class Label : public IStylableWidget<Label>
	{
	public:
		Label(StringView aText = "", ImFont* aFont = nullptr) noexcept;

		NO_DISCARD String GetText() const noexcept;

		virtual void OnRender() noexcept override;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		Label* SetHighlightedSubstring(StringView aText) noexcept;
		Label* SetText(StringView aText) noexcept;
		Label* SetText(Callback<String()>&& aTextCallback) noexcept;
	private:
		void RenderHighlight() noexcept;
		void RenderText() noexcept;
	private:
		Callback<String()> m_TextProvider;
		String m_HighlightedSubstring;
	};

}