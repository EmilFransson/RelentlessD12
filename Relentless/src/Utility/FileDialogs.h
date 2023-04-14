#pragma once

namespace Relentless
{
	class FileDialogs
	{
	public:
		[[nodiscard]] static std::string OpenFile(const char* filter) noexcept;
	private:

	};
}