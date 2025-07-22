#pragma once

namespace Relentless
{
	namespace StringUtils
	{
		NO_DISCARD String ConvertFromWide(const std::wstring& input) noexcept;
		NO_DISCARD std::wstring ConvertToWide(const String& input) noexcept;
		NO_DISCARD std::vector<String> Split(const String& input, char delimiter) noexcept;
		NO_DISCARD String ToLower(const String& input) noexcept;
	}
}