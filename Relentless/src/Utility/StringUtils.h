#pragma once

namespace Relentless
{
	namespace StringUtils
	{
		[[nodiscard]] std::string ConvertFromWide(const std::wstring& input) noexcept;
		[[nodiscard]] std::wstring ConvertToWide(const std::string& input) noexcept;
		[[nodiscard]] std::vector<std::string> Split(const std::string& input, char delimiter) noexcept;
	}
}