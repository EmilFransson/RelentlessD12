#pragma once

namespace Relentless
{
	class FileDialogs
	{
	public:
		[[nodiscard]] static std::vector<std::string> OpenFile(const char* filter) noexcept;
		[[nodiscard]] static std::string SaveFile(const char* filter) noexcept;
	};
}