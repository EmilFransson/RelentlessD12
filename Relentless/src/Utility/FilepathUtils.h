#pragma once

namespace Relentless
{
	class FilepathUtils
	{
	public:
		static [[nodiscard]] std::string ExtractFilename(const std::filesystem::path& filepath) noexcept;
		static [[nodiscard]] std::string ExtractFilenameWithoutExtension(const std::filesystem::path& filepath) noexcept;
		static [[nodiscard]] std::string ExtractExtension(const std::filesystem::path& filepath) noexcept;
		static [[nodiscard]] std::filesystem::path Combine(const std::filesystem::path& basePath, const std::filesystem::path& pathToAppend) noexcept;
		static void Normalize(std::filesystem::path& basePath) noexcept;
		static [[nodiscard]] bool HasExtension(const std::filesystem::path& path) noexcept;
		static void SetExtension(std::filesystem::path& path, const std::string& extension) noexcept;
		static [[nodiscard]] std::string SanitizeFileName(const std::string& fileName) noexcept;
		static [[nodiscard]] bool IsDirectory(const std::filesystem::path& filepath) noexcept;
	};
}