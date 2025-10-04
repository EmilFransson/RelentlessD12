#pragma once

namespace Relentless
{
	class FilepathUtils
	{
	public:
		static NO_DISCARD String ExtractFilename(const Path& aFilepath) noexcept;
		static NO_DISCARD String ExtractFilenameWithoutExtension(const Path& aFilepath) noexcept;
		static NO_DISCARD String ExtractExtension(const Path& aFilepath) noexcept;
		static NO_DISCARD Path Combine(const Path& aBasePath, const Path& aPathToAppend) noexcept;
		static NO_DISCARD String CombineDisplay(const Path& aBasePath, const Path& aPathToAppend) noexcept;
		static void Normalize(Path& basePath) noexcept;
		static NO_DISCARD bool HasExtension(const Path& aFilePath) noexcept;
		static void SetExtension(Path& aFilePath, const String& aExtension) noexcept;
		static NO_DISCARD String SanitizeFileName(const String& aFileName) noexcept;
		static NO_DISCARD bool IsDirectory(const Path& aFilepath) noexcept;
	};
}