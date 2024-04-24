#pragma once

namespace Relentless
{
	class FilepathUtils
	{
	public:
		static [[nodiscard]] std::string ExtractExtension(const std::filesystem::path& filepath) noexcept;
		static [[nodiscard]] std::filesystem::path Combine(const std::filesystem::path& basePath, const std::filesystem::path& pathToAppend) noexcept;
	};
}