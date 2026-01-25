#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Tooltip : public RefCounted<Tooltip>
	{
	public:
		Tooltip(std::string_view text = "") noexcept;
		virtual ~Tooltip() noexcept = default;

		virtual void OnRender() noexcept;

		NO_DISCARD const String& GetText() const noexcept;
		void SetText(std::string_view text) noexcept;

	private:
		String m_Text;
	};
}