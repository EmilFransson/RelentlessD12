#include "Utility.h"

#include "Utility/StringUtils.h"

namespace Relentless::Platform
{
	bool IsReservedDeviceName(StringView aName) noexcept
	{
		// strip extension: "CON.txt" → "CON"
		const size_t dot = aName.find('.');
		const StringView stem = (dot == StringView::npos) ? aName : aName.substr(0, dot);

		// CON PRN AUX NUL, COM1–COM9, LPT1–LPT9 (and COM0/LPT0 on newer Windows)
		static constexpr std::array<StringView, 4> simple = { "CON", "PRN", "AUX", "NUL" };

		for (const StringView r : simple)
			if (StringUtils::EqualsIgnoreCase(stem, r))
				return true;

		// COM1..9 / LPT1..9 — three chars, last is a digit 1-9
		if (stem.size() == 4u && (StringUtils::EqualsIgnoreCase(stem.substr(0, 3), "COM") || StringUtils::EqualsIgnoreCase(stem.substr(0, 3), "LPT")) && stem[3] >= '1' && stem[3] <= '9')
		{
			return true;
		}

		return false;
	}

	bool IsValidFileName(StringView aName) noexcept
	{
		if (aName.empty())
			return false;

		// 1. Illegal characters: < > : " / \ | ? *  and all control chars (0x00–0x1F)
		constexpr StringView illegal = "<>:\"/\\|?*";
		for (const char c : aName)
		{
			if (static_cast<unsigned char>(c) < 0x20u)   // control characters
				return false;
			if (illegal.find(c) != StringView::npos)
				return false;
		}

		// 2. Trailing dot or space — Windows silently strips these, so "Foo." becomes
		//    "Foo" on disk and your namespace desyncs from reality.
		if (aName.back() == ' ' || aName.back() == '.')
			return false;

		// 3. Leading space is also trouble (and usually a typo). Reject.
		if (aName.front() == ' ')
			return false;

		// 4. Reserved device names — case-insensitive, with or without an extension.
		//    "CON", "con", "CON.txt" are ALL reserved. This is the one everyone forgets.
		if (IsReservedDeviceName(aName))
			return false;

		return true;
	}
}