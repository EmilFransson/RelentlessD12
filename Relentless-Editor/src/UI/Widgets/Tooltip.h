#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Tooltip : public RefCounted<Tooltip>
	{
	public:
		Tooltip(StringView aText = "") noexcept;
		virtual ~Tooltip() noexcept = default;

		virtual void OnRender() noexcept;

		NO_DISCARD const String& GetText() const noexcept;
		void SetText(StringView aText) noexcept;
	private:
		String m_Text;
	};
}