#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Tooltip : public RefCounted<Tooltip>
	{
	public:
		Tooltip(StringView aText = "") noexcept;
		virtual ~Tooltip() noexcept = default;

		NO_DISCARD String GetText() const noexcept;

		virtual void OnRender() noexcept;

		void SetText(StringView aText) noexcept;
		void SetText(Callback<String()>&& aTextCallback) noexcept;
	private:
		Callback<String()> m_TooltipProvider;
	};
}