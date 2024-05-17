#pragma once
namespace Relentless
{
	class File
	{
	public:
		static [[nodiscard]] bool Exists(const std::filesystem::path& path) noexcept;
		static [[nodiscard]] bool ExistsDir(const std::filesystem::path& path) noexcept;
	private:
	};
}