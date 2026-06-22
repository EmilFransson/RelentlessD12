#pragma once
#include <Relentless.h>
#include "IStylableWidget.h"

namespace Relentless
{
	enum class ETextAlign : uint8 { Left = 0u, Center, Right };
	
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
		Label* SetTextAlign(ETextAlign aAlign) noexcept { m_TextAlign = aAlign; return this; }
		Label* SetWrap(bool aWrap) noexcept;
	private:
		void RenderHighlight() noexcept;
		//void RenderText() noexcept;
		void RenderText(float aWrapWidth, ImVec2 aOrigin, Vector2 aPad, float aFramePadY) noexcept;
	private:
		struct LineLayout { const char* begin; const char* end; float x; float y; };

		void BuildLineLayout(const String& aText, float aWrapWidth, ImVec2 aOrigin) noexcept;

		ETextAlign m_TextAlign = ETextAlign::Left;
		std::vector<LineLayout> m_Lines;

		Callback<String()> m_TextProvider;
		String m_HighlightedSubstring;
		bool m_Wrap = false;
	};

}