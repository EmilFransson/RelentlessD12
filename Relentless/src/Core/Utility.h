#pragma once
namespace Relentless
{
	[[nodiscard]] std::string ConvertWstringToString(std::wstring const& wstr) noexcept;
	[[nodiscard]] std::wstring ConvertStringToWstring(const std::string& string) noexcept;
}