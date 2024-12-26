#pragma once

namespace Relentless
{
	namespace StringUtils
	{
		[[nodiscard]] std::vector<std::string> Split(const std::string& input, char delimiter) noexcept;
	}
}