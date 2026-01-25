#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	class RLS_API FilepathUtils
	{
	public:
		static bool CreateDirectoryTree(const Path& aPath) noexcept;
		NO_DISCARD static String ExtractFilename(const Path& aFilepath) noexcept;
		NO_DISCARD static String ExtractFilenameWithoutExtension(const Path& aFilepath) noexcept;
		NO_DISCARD static String ExtractExtension(const Path& aFilepath) noexcept;
		NO_DISCARD static Path Combine(const Path& aBasePath, const Path& aPathToAppend) noexcept;
		NO_DISCARD static String CombineDisplay(const Path& aBasePath, const Path& aPathToAppend) noexcept;
		static void Normalize(Path& basePath) noexcept;
		NO_DISCARD static bool HasExtension(const Path& aFilePath) noexcept;
		static bool SetFileHidden(const Path& aFilepath) noexcept;
		static void SetExtension(Path& aFilePath, const String& aExtension) noexcept;
		NO_DISCARD static String SanitizeFileName(const String& aFileName) noexcept;
		NO_DISCARD static bool IsDirectory(const Path& aFilepath) noexcept;
	};
}